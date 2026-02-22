#pragma once
// =============================================================================
// payment_processor.hpp — Multi-Gateway Payment Processing Service
// Design Pattern: Strategy (PaymentGateway)
// Features: Partial/overpayment, retry with exponential backoff, refunds
// =============================================================================
#include "../core/snowflake.hpp"
#include "../models/invoice.hpp"
#include "../models/payment.hpp"
#include "../repository/invoice_repository.hpp"
#include "../repository/payment_repository.hpp"
#include <chrono>
#include <ctime>
#include <map>
#include <memory>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace billing {
namespace service {

// ---------------------------------------------------------------------------
// Strategy: Gateway interface
// ---------------------------------------------------------------------------
struct PaymentGateway {
  virtual ~PaymentGateway() = default;
  virtual std::string name() const = 0;
  // Returns GatewayResult
  virtual models::GatewayResult process(double amount,
                                        const std::string &ref) = 0;
};

// Credit Card Gateway simulation
struct CreditCardGateway : PaymentGateway {
  std::mt19937 rng{std::random_device{}()};
  std::string name() const override { return "CreditCard"; }
  models::GatewayResult process(double amount, const std::string &) override {
    std::uniform_int_distribution<> dist(1, 100);
    int r = dist(rng);
    if (r <= 90)
      return models::GatewayResult::SUCCESS;
    if (r <= 94)
      return models::GatewayResult::INSUFFICIENT_FUNDS;
    if (r <= 97)
      return models::GatewayResult::CARD_DECLINED;
    if (r <= 99)
      return models::GatewayResult::NETWORK_ERROR;
    return models::GatewayResult::FRAUD_DETECTED;
  }
};

// Bank Transfer Gateway simulation
struct BankTransferGateway : PaymentGateway {
  std::mt19937 rng{std::random_device{}()};
  std::string name() const override { return "BankTransfer"; }
  models::GatewayResult process(double amount, const std::string &) override {
    std::uniform_int_distribution<> dist(1, 100);
    int r = dist(rng);
    if (r <= 95)
      return models::GatewayResult::SUCCESS;
    if (r <= 98)
      return models::GatewayResult::NETWORK_ERROR;
    return models::GatewayResult::TIMEOUT;
  }
};

// Wallet Gateway simulation
struct WalletGateway : PaymentGateway {
  std::mt19937 rng{std::random_device{}()};
  std::string name() const override { return "Wallet"; }
  models::GatewayResult process(double amount, const std::string &) override {
    std::uniform_int_distribution<> dist(1, 100);
    int r = dist(rng);
    if (r <= 97)
      return models::GatewayResult::SUCCESS;
    if (r <= 99)
      return models::GatewayResult::INSUFFICIENT_FUNDS;
    return models::GatewayResult::TIMEOUT;
  }
};

// ---------------------------------------------------------------------------
// PaymentProcessor service
// ---------------------------------------------------------------------------
class PaymentProcessor {
public:
  static constexpr int MAX_RETRIES = 5;
  static constexpr double BACKOFF_BASE_MS = 200.0;             // 200ms base
  static constexpr double OVERPAYMENT_CREDIT_THRESHOLD = 0.50; // $0.50

  PaymentProcessor(repository::InvoiceRepository &inv_repo,
                   repository::PaymentRepository &pay_repo)
      : inv_repo_(inv_repo), pay_repo_(pay_repo) {
    // Register default gateways
    gateways_[models::PaymentMethod::CREDIT_CARD] =
        std::make_unique<CreditCardGateway>();
    gateways_[models::PaymentMethod::BANK_TRANSFER] =
        std::make_unique<BankTransferGateway>();
    gateways_[models::PaymentMethod::WALLET] =
        std::make_unique<WalletGateway>();
  }

  struct PaymentResult {
    bool success;
    models::Payment payment;
    std::string message;
    double credit_balance; // overpayment credit
  };

  // Main process function — with retry + exponential backoff
  PaymentResult process_payment(int64_t invoice_id, int64_t customer_id,
                                double amount, models::PaymentMethod method,
                                const std::string &notes = "") {
    auto inv_opt = inv_repo_.find_by_id(invoice_id);
    if (!inv_opt)
      throw std::runtime_error("Invoice not found: " +
                               std::to_string(invoice_id));
    auto &inv = *inv_opt;

    if (inv.status == models::InvoiceStatus::PAID)
      throw std::runtime_error("Invoice already paid");
    if (inv.status == models::InvoiceStatus::CANCELLED)
      throw std::runtime_error("Invoice is cancelled");

    // Build payment record
    models::Payment p;
    p.id = core::generate_id();
    p.invoice_id = invoice_id;
    p.customer_id = customer_id;
    p.method = method;
    p.status = models::PaymentStatus::PENDING;
    p.amount = amount;
    p.refund_amount = 0.0;
    p.currency = inv.currency;
    p.notes = notes;
    p.retry_count = 0;
    p.fraud_flagged = false;
    p.created_at = std::time(nullptr);
    p.completed_at = 0;
    p.gateway_ref = "REF-" + std::to_string(core::generate_id());

    // Try payment with exponential backoff
    auto *gw = get_gateway(method);
    models::GatewayResult gw_result = models::GatewayResult::NETWORK_ERROR;
    for (int attempt = 0; attempt <= MAX_RETRIES; ++attempt) {
      gw_result = gw->process(amount, p.gateway_ref);
      if (gw_result == models::GatewayResult::SUCCESS)
        break;
      if (gw_result == models::GatewayResult::FRAUD_DETECTED ||
          gw_result == models::GatewayResult::CARD_DECLINED ||
          gw_result == models::GatewayResult::INSUFFICIENT_FUNDS) {
        break; // No point retrying non-transient errors
      }
      if (attempt < MAX_RETRIES) {
        // Exponential backoff: base * 2^attempt ms
        double wait_ms = BACKOFF_BASE_MS * (1 << attempt);
        std::this_thread::sleep_for(
            std::chrono::milliseconds(static_cast<int>(wait_ms)));
        p.retry_count++;
      }
    }

    PaymentResult result;
    result.credit_balance = 0.0;

    if (gw_result == models::GatewayResult::SUCCESS) {
      p.status = models::PaymentStatus::COMPLETED;
      p.completed_at = std::time(nullptr);
      result.success = true;
      result.message = "Payment successful via " + gw->name();

      // Update invoice amount_paid
      inv.amount_paid += amount;
      double due = inv.total_amount - inv.amount_paid;

      if (inv.amount_paid >= inv.total_amount) {
        // Handle overpayment
        if (-due > OVERPAYMENT_CREDIT_THRESHOLD)
          result.credit_balance = -due; // positive credit
        inv.amount_paid = inv.total_amount;
        inv.status = models::InvoiceStatus::PAID;
        inv.paid_date = std::time(nullptr);
      } else {
        inv.status = models::InvoiceStatus::PARTIALLY_PAID;
        p.status = models::PaymentStatus::PARTIAL;
      }
      inv_repo_.update(inv);
    } else {
      p.status = models::PaymentStatus::FAILED;
      if (gw_result == models::GatewayResult::FRAUD_DETECTED)
        p.fraud_flagged = true;
      result.success = false;
      result.message = gateway_result_to_string(gw_result);
    }

    result.payment = p;
    pay_repo_.save(p);
    return result;
  }

  // Process refund
  struct RefundResult {
    bool success;
    std::string message;
    models::Refund refund;
  };

  RefundResult process_refund(int64_t payment_id, double amount,
                              const std::string &reason) {
    auto pay_opt = pay_repo_.find_by_id(payment_id);
    if (!pay_opt)
      throw std::runtime_error("Payment not found");
    auto pay = *pay_opt;
    if (pay.status != models::PaymentStatus::COMPLETED &&
        pay.status != models::PaymentStatus::PARTIAL)
      throw std::runtime_error("Payment not eligible for refund");
    if (amount > pay.amount)
      throw std::runtime_error("Refund exceeds payment amount");

    models::Refund ref;
    ref.id = core::generate_id();
    ref.payment_id = payment_id;
    ref.invoice_id = pay.invoice_id;
    ref.amount = amount;
    ref.reason = reason;
    ref.created_at = std::time(nullptr);

    pay.refund_amount += amount;
    pay.status = models::PaymentStatus::REFUNDED;
    pay_repo_.update(pay);

    // Update invoice status back
    auto inv_opt = inv_repo_.find_by_id(pay.invoice_id);
    if (inv_opt) {
      auto inv = *inv_opt;
      inv.amount_paid -= amount;
      if (inv.amount_paid <= 0) {
        inv.amount_paid = 0;
        inv.status = models::InvoiceStatus::REFUNDED;
      } else {
        inv.status = models::InvoiceStatus::PARTIALLY_PAID;
      }
      inv_repo_.update(inv);
    }

    return {true, "Refund of $" + std::to_string(amount) + " processed", ref};
  }

  std::vector<models::Payment> payment_history(int64_t customer_id) const {
    return pay_repo_.find_by_customer(customer_id);
  }

private:
  PaymentGateway *get_gateway(models::PaymentMethod m) {
    auto it = gateways_.find(m);
    if (it == gateways_.end())
      throw std::runtime_error("No gateway for method");
    return it->second.get();
  }

  static std::string gateway_result_to_string(models::GatewayResult r) {
    switch (r) {
    case models::GatewayResult::SUCCESS:
      return "Success";
    case models::GatewayResult::INSUFFICIENT_FUNDS:
      return "Insufficient funds";
    case models::GatewayResult::CARD_DECLINED:
      return "Card declined";
    case models::GatewayResult::NETWORK_ERROR:
      return "Network error (retries exhausted)";
    case models::GatewayResult::FRAUD_DETECTED:
      return "Fraud detected";
    case models::GatewayResult::TIMEOUT:
      return "Timeout (retries exhausted)";
    }
    return "Unknown error";
  }

  repository::InvoiceRepository &inv_repo_;
  repository::PaymentRepository &pay_repo_;
  std::map<models::PaymentMethod, std::unique_ptr<PaymentGateway>> gateways_;
};

} // namespace service
} // namespace billing
