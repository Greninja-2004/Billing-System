#pragma once
// =============================================================================
// invoice_repository.hpp — File-based Invoice Persistence
// Binary serialization with B+ Tree indexing + LRU Cache
// =============================================================================
#include "../core/bplus_tree.hpp"
#include "../core/lru_cache.hpp"
#include "../models/invoice.hpp"
#include <algorithm>
#include <fstream>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace billing::repository {

class InvoiceRepository {
public:
  explicit InvoiceRepository(const std::string &data_dir)
      : data_file_(data_dir + "/invoices.bin"), index_(), cache_(512) {
    load_all();
  }

  void save(const models::Invoice &inv) {
    std::lock_guard<std::mutex> lock(mutex_);
    store_[inv.id] = inv;
    index_.insert(inv.id, inv.id);
    cache_.put(inv.id, inv);
    flush();
  }

  std::optional<models::Invoice> find_by_id(int64_t id) {
    auto cached = cache_.get(id);
    if (cached)
      return cached;
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = store_.find(id);
    if (it == store_.end())
      return std::nullopt;
    cache_.put(id, it->second);
    return it->second;
  }

  bool update(const models::Invoice &inv) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (store_.find(inv.id) == store_.end())
      return false;
    store_[inv.id] = inv;
    cache_.put(inv.id, inv);
    flush();
    return true;
  }

  bool remove(int64_t id) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (store_.erase(id) == 0)
      return false;
    index_.remove(id);
    cache_.evict(id);
    flush();
    return true;
  }

  std::vector<models::Invoice> find_all() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<models::Invoice> result;
    result.reserve(store_.size());
    for (auto &[id, inv] : store_)
      result.push_back(inv);
    return result;
  }

  std::vector<models::Invoice> find_by_customer(int64_t customer_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<models::Invoice> result;
    for (auto &[id, inv] : store_)
      if (inv.customer_id == customer_id)
        result.push_back(inv);
    return result;
  }

  std::vector<models::Invoice>
  find_by_status(models::InvoiceStatus status) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<models::Invoice> result;
    for (auto &[id, inv] : store_)
      if (inv.status == status)
        result.push_back(inv);
    return result;
  }

  std::vector<models::Invoice> find_overdue() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<models::Invoice> result;
    for (auto &[id, inv] : store_)
      if (inv.is_overdue())
        result.push_back(inv);
    return result;
  }

  std::size_t count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return store_.size();
  }

private:
  static void write_string(std::ofstream &f, const std::string &s) {
    std::size_t len = s.size();
    f.write(reinterpret_cast<const char *>(&len), sizeof(len));
    f.write(s.data(), static_cast<std::streamsize>(len));
  }
  static void read_string(std::ifstream &f, std::string &s) {
    std::size_t len = 0;
    f.read(reinterpret_cast<char *>(&len), sizeof(len));
    s.resize(len);
    f.read(s.data(), static_cast<std::streamsize>(len));
  }

  void write_invoice(std::ofstream &f, const models::Invoice &inv) {
    f.write(reinterpret_cast<const char *>(&inv.id), sizeof(inv.id));
    f.write(reinterpret_cast<const char *>(&inv.customer_id),
            sizeof(inv.customer_id));
    f.write(reinterpret_cast<const char *>(&inv.parent_invoice_id),
            sizeof(inv.parent_invoice_id));
    write_string(f, inv.invoice_number);
    f.write(reinterpret_cast<const char *>(&inv.type), sizeof(inv.type));
    f.write(reinterpret_cast<const char *>(&inv.period), sizeof(inv.period));
    f.write(reinterpret_cast<const char *>(&inv.status), sizeof(inv.status));

    // Line items
    std::size_t li_count = inv.line_items.size();
    f.write(reinterpret_cast<const char *>(&li_count), sizeof(li_count));
    for (auto &li : inv.line_items) {
      write_string(f, li.description);
      f.write(reinterpret_cast<const char *>(&li.quantity),
              sizeof(li.quantity));
      f.write(reinterpret_cast<const char *>(&li.unit_price),
              sizeof(li.unit_price));
    }

    f.write(reinterpret_cast<const char *>(&inv.subtotal),
            sizeof(inv.subtotal));
    f.write(reinterpret_cast<const char *>(&inv.discount_amount),
            sizeof(inv.discount_amount));
    f.write(reinterpret_cast<const char *>(&inv.tax_amount),
            sizeof(inv.tax_amount));
    f.write(reinterpret_cast<const char *>(&inv.total_amount),
            sizeof(inv.total_amount));
    f.write(reinterpret_cast<const char *>(&inv.amount_paid),
            sizeof(inv.amount_paid));
    write_string(f, inv.currency);
    write_string(f, inv.jurisdiction);
    write_string(f, inv.notes);
    f.write(reinterpret_cast<const char *>(&inv.issue_date),
            sizeof(inv.issue_date));
    f.write(reinterpret_cast<const char *>(&inv.due_date),
            sizeof(inv.due_date));
    f.write(reinterpret_cast<const char *>(&inv.paid_date),
            sizeof(inv.paid_date));
    f.write(reinterpret_cast<const char *>(&inv.next_billing_date),
            sizeof(inv.next_billing_date));
    f.write(reinterpret_cast<const char *>(&inv.period_start),
            sizeof(inv.period_start));
    f.write(reinterpret_cast<const char *>(&inv.period_end),
            sizeof(inv.period_end));
  }

  void read_invoice(std::ifstream &f, models::Invoice &inv) {
    f.read(reinterpret_cast<char *>(&inv.id), sizeof(inv.id));
    f.read(reinterpret_cast<char *>(&inv.customer_id), sizeof(inv.customer_id));
    f.read(reinterpret_cast<char *>(&inv.parent_invoice_id),
           sizeof(inv.parent_invoice_id));
    read_string(f, inv.invoice_number);
    f.read(reinterpret_cast<char *>(&inv.type), sizeof(inv.type));
    f.read(reinterpret_cast<char *>(&inv.period), sizeof(inv.period));
    f.read(reinterpret_cast<char *>(&inv.status), sizeof(inv.status));

    std::size_t li_count = 0;
    f.read(reinterpret_cast<char *>(&li_count), sizeof(li_count));
    inv.line_items.resize(li_count);
    for (auto &li : inv.line_items) {
      read_string(f, li.description);
      f.read(reinterpret_cast<char *>(&li.quantity), sizeof(li.quantity));
      f.read(reinterpret_cast<char *>(&li.unit_price), sizeof(li.unit_price));
    }

    f.read(reinterpret_cast<char *>(&inv.subtotal), sizeof(inv.subtotal));
    f.read(reinterpret_cast<char *>(&inv.discount_amount),
           sizeof(inv.discount_amount));
    f.read(reinterpret_cast<char *>(&inv.tax_amount), sizeof(inv.tax_amount));
    f.read(reinterpret_cast<char *>(&inv.total_amount),
           sizeof(inv.total_amount));
    f.read(reinterpret_cast<char *>(&inv.amount_paid), sizeof(inv.amount_paid));
    read_string(f, inv.currency);
    read_string(f, inv.jurisdiction);
    read_string(f, inv.notes);
    f.read(reinterpret_cast<char *>(&inv.issue_date), sizeof(inv.issue_date));
    f.read(reinterpret_cast<char *>(&inv.due_date), sizeof(inv.due_date));
    f.read(reinterpret_cast<char *>(&inv.paid_date), sizeof(inv.paid_date));
    f.read(reinterpret_cast<char *>(&inv.next_billing_date),
           sizeof(inv.next_billing_date));
    f.read(reinterpret_cast<char *>(&inv.period_start),
           sizeof(inv.period_start));
    f.read(reinterpret_cast<char *>(&inv.period_end), sizeof(inv.period_end));
  }

  void load_all() {
    std::ifstream f(data_file_, std::ios::binary);
    if (!f.is_open())
      return;
    std::size_t count = 0;
    f.read(reinterpret_cast<char *>(&count), sizeof(count));
    for (std::size_t i = 0; i < count; ++i) {
      models::Invoice inv;
      read_invoice(f, inv);
      store_[inv.id] = inv;
      index_.insert(inv.id, inv.id);
    }
  }

  void flush() {
    std::ofstream f(data_file_, std::ios::binary | std::ios::trunc);
    if (!f.is_open())
      throw std::runtime_error("Cannot open invoice data file for writing");
    std::size_t count = store_.size();
    f.write(reinterpret_cast<const char *>(&count), sizeof(count));
    for (auto &[id, inv] : store_)
      write_invoice(f, inv);
  }

  std::string data_file_;
  std::unordered_map<int64_t, models::Invoice> store_;
  core::BPlusTree<int64_t, int64_t> index_;
  mutable core::LRUCache<int64_t, models::Invoice> cache_;
  mutable std::mutex mutex_;
};

} // namespace billing::repository
