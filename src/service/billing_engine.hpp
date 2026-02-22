#pragma once
// =============================================================================
// billing_engine.hpp — Invoice & Billing Engine Service
// Design Pattern: Factory (InvoiceFactory), Observer (billing events)
// Multi-threading: std::thread for concurrent batch generation
// =============================================================================
#include "../core/min_heap.hpp"
#include "../core/snowflake.hpp"
#include "../models/customer.hpp"
#include "../models/invoice.hpp"
#include "../repository/customer_repository.hpp"
#include "../repository/invoice_repository.hpp"
#include "discount_engine.hpp"
#include "tax_engine.hpp"
#include <atomic>
#include <ctime>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace billing {
namespace service {

// Observer interface for billing events
struct BillingObserver {
  virtual ~BillingObserver() = default;
  virtual void on_invoice_created(const models::Invoice &inv) = 0;
  virtual void on_invoice_paid(const models::Invoice &inv) = 0;
  virtual void on_invoice_overdue(const models::Invoice &inv) = 0;
};

// Invoice creation request
struct InvoiceRequest {
  int64_t customer_id;
  models::InvoiceType type;
  models::RecurringPeriod period = models::RecurringPeriod::NONE;
  std::vector<models::LineItem> line_items;
  std::string currency = "USD";
  std::string notes;
  int64_t parent_invoice_id = 0;
  // For prorated billing
  std::time_t period_start = 0;
  std::time_t period_end = 0;
  int due_days = 30; // days from issue to due
};

// Named comparator for invoice due_date ordering (C++17-compatible)
struct InvoiceDueCompare {
  bool operator()(const models::Invoice &a, const models::Invoice &b) const {
    return a.due_date < b.due_date;
  }
};

class BillingEngine {
public:
  BillingEngine(repository::InvoiceRepository &inv_repo,
                repository::CustomerRepository &cust_repo, DiscountEngine &disc,
                TaxEngine &tax)
      : inv_repo_(inv_repo), cust_repo_(cust_repo), discount_(disc), tax_(tax),
        invoice_counter_(1) {}

  void add_observer(BillingObserver *obs) {
    std::lock_guard<std::mutex> lock(obs_mutex_);
    observers_.push_back(obs);
  }

  // ==========================================================================
  // Factory method — create any invoice type
  // ==========================================================================
  models::Invoice create_invoice(const InvoiceRequest &req) {
    auto cust_opt = cust_repo_.find_by_id(req.customer_id);
    if (!cust_opt)
      throw std::runtime_error("Customer not found");
    const models::Customer &cust = *cust_opt;

    models::Invoice inv;
    inv.id = core::generate_id();
    inv.customer_id = req.customer_id;
    inv.parent_invoice_id = req.parent_invoice_id;
    inv.type = req.type;
    inv.period = req.period;
    inv.status = models::InvoiceStatus::PENDING;
    inv.line_items = req.line_items;
    inv.currency = req.currency;
    inv.notes = req.notes;
    inv.period_start = req.period_start;
    inv.period_end = req.period_end;
    inv.amount_paid = 0.0;
    inv.paid_date = 0;

    // Generate human-readable invoice number
    inv.invoice_number = generate_invoice_number();

    // Compute subtotal from line items
    inv.subtotal = 0.0;
    for (const auto &li : inv.line_items)
      inv.subtotal += li.total();

    // Handle proration
    if (req.type == models::InvoiceType::PRORATED && req.period_start &&
        req.period_end) {
      double period_days =
          std::difftime(req.period_end, req.period_start) / 86400.0;
      double days_in_month = 30.0;
      double prorate_factor = period_days / days_in_month;
      inv.subtotal *= prorate_factor;
    }

    // Determine tax jurisdiction
    inv.jurisdiction = TaxEngine::jurisdiction(cust.country, cust.state);

    // Apply discounts
    inv.discount_amount = discount_.apply(inv.subtotal, cust, inv);

    // Compute tax on discounted subtotal
    double taxable = inv.subtotal - inv.discount_amount;
    auto tax_result = tax_.compute(taxable, inv.jurisdiction);
    inv.tax_amount = tax_result.total_tax;

    // Final total
    inv.total_amount = inv.subtotal - inv.discount_amount + inv.tax_amount;

    // Dates
    inv.issue_date = std::time(nullptr);
    inv.due_date =
        inv.issue_date + static_cast<std::time_t>(req.due_days) * 86400;

    // Next billing date for recurring
    if (req.type == models::InvoiceType::RECURRING)
      inv.next_billing_date = compute_next_billing(inv.issue_date, req.period);
    else
      inv.next_billing_date = 0;

    // Push to payment scheduler
    scheduler_.push(inv);

    inv_repo_.save(inv);
    notify_created(inv);
    return inv;
  }

  // ==========================================================================
  // Batch generation with multi-threading
  // ==========================================================================
  std::vector<models::Invoice>
  batch_create(const std::vector<InvoiceRequest> &requests,
               int num_threads = 4) {
    std::vector<models::Invoice> results(requests.size());
    std::mutex result_mutex;
    std::vector<std::thread> threads;
    std::atomic<std::size_t> idx{0};

    auto worker = [&]() {
      while (true) {
        std::size_t i = idx.fetch_add(1);
        if (i >= requests.size())
          break;
        try {
          auto inv = create_invoice(requests[i]);
          std::lock_guard<std::mutex> lk(result_mutex);
          results[i] = inv;
        } catch (...) {
        }
      }
    };

    for (int t = 0; t < num_threads; ++t)
      threads.emplace_back(worker);
    for (auto &th : threads)
      th.join();

    return results;
  }

  // ==========================================================================
  // Recurring: generate next invoice in chain
  // ==========================================================================
  std::optional<models::Invoice>
  generate_next_recurring(const models::Invoice &parent) {
    if (parent.type != models::InvoiceType::RECURRING)
      return std::nullopt;
    if (parent.next_billing_date == 0)
      return std::nullopt;

    InvoiceRequest req;
    req.customer_id = parent.customer_id;
    req.type = models::InvoiceType::RECURRING;
    req.period = parent.period;
    req.line_items = parent.line_items;
    req.currency = parent.currency;
    req.notes = "Auto-recurring from INV " + parent.invoice_number;
    req.parent_invoice_id = parent.id;
    req.due_days = 30;

    return create_invoice(req);
  }

  // Mark invoice as paid (or partially paid)
  bool mark_paid(int64_t invoice_id, double amount_paid) {
    auto opt = inv_repo_.find_by_id(invoice_id);
    if (!opt)
      return false;
    auto inv = *opt;

    inv.amount_paid += amount_paid;
    if (inv.amount_paid >= inv.total_amount) {
      inv.amount_paid = inv.total_amount;
      inv.status = models::InvoiceStatus::PAID;
      inv.paid_date = std::time(nullptr);
      notify_paid(inv);
    } else {
      inv.status = models::InvoiceStatus::PARTIALLY_PAID;
    }
    inv_repo_.update(inv);
    return true;
  }

  // Scan and flag overdue invoices — O(n)
  int flag_overdue() {
    auto all = inv_repo_.find_all();
    int count = 0;
    for (auto &inv : all) {
      if (inv.status == models::InvoiceStatus::PENDING && inv.is_overdue()) {
        inv.status = models::InvoiceStatus::OVERDUE;
        inv_repo_.update(inv);
        notify_overdue(inv);
        count++;
      }
    }
    return count;
  }

  // Next payment due (from min-heap scheduler)
  std::optional<models::Invoice> next_due() {
    if (scheduler_.empty())
      return std::nullopt;
    return scheduler_.pop();
  }

  std::size_t pending_in_scheduler() const { return scheduler_.size(); }

private:
  std::string generate_invoice_number() {
    std::lock_guard<std::mutex> lock(counter_mutex_);
    std::time_t now = std::time(nullptr);
    std::tm *t = std::localtime(&now);
    char buf[32];
    std::strftime(buf, sizeof(buf), "INV-%Y%m", t);
    std::string num = std::to_string(invoice_counter_++);
    while (num.size() < 4)
      num = "0" + num;
    return std::string(buf) + num;
  }

  static std::time_t compute_next_billing(std::time_t from,
                                          models::RecurringPeriod period) {
    switch (period) {
    case models::RecurringPeriod::DAILY:
      return from + 86400;
    case models::RecurringPeriod::WEEKLY:
      return from + 7 * 86400;
    case models::RecurringPeriod::MONTHLY:
      return from + 30 * 86400;
    case models::RecurringPeriod::YEARLY:
      return from + 365 * 86400;
    default:
      return 0;
    }
  }

  void notify_created(const models::Invoice &inv) {
    std::lock_guard<std::mutex> lock(obs_mutex_);
    for (auto *obs : observers_)
      obs->on_invoice_created(inv);
  }
  void notify_paid(const models::Invoice &inv) {
    std::lock_guard<std::mutex> lock(obs_mutex_);
    for (auto *obs : observers_)
      obs->on_invoice_paid(inv);
  }
  void notify_overdue(const models::Invoice &inv) {
    std::lock_guard<std::mutex> lock(obs_mutex_);
    for (auto *obs : observers_)
      obs->on_invoice_overdue(inv);
  }

  repository::InvoiceRepository &inv_repo_;
  repository::CustomerRepository &cust_repo_;
  DiscountEngine &discount_;
  TaxEngine &tax_;

  // Min-heap: schedules invoices by due_date (C++17-compatible named
  // comparator)
  core::MinHeap<models::Invoice, InvoiceDueCompare> scheduler_;

  std::vector<BillingObserver *> observers_;
  std::mutex obs_mutex_;

  std::atomic<int> invoice_counter_;
  std::mutex counter_mutex_;
};

} // namespace service
} // namespace billing
