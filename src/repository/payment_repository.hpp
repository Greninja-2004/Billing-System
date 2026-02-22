#pragma once
// =============================================================================
// payment_repository.hpp — File-based Payment Persistence
// =============================================================================
#include "../core/lru_cache.hpp"
#include "../models/payment.hpp"
#include <fstream>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace billing::repository {

class PaymentRepository {
public:
  explicit PaymentRepository(const std::string &data_dir)
      : data_file_(data_dir + "/payments.bin"), cache_(256) {
    load_all();
  }

  void save(const models::Payment &p) {
    std::lock_guard<std::mutex> lock(mutex_);
    store_[p.id] = p;
    cache_.put(p.id, p);
    flush();
  }

  std::optional<models::Payment> find_by_id(int64_t id) {
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

  bool update(const models::Payment &p) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (store_.find(p.id) == store_.end())
      return false;
    store_[p.id] = p;
    cache_.put(p.id, p);
    flush();
    return true;
  }

  std::vector<models::Payment> find_by_invoice(int64_t invoice_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<models::Payment> result;
    for (auto &[id, p] : store_)
      if (p.invoice_id == invoice_id)
        result.push_back(p);
    return result;
  }

  std::vector<models::Payment> find_by_customer(int64_t customer_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<models::Payment> result;
    for (auto &[id, p] : store_)
      if (p.customer_id == customer_id)
        result.push_back(p);
    return result;
  }

  std::vector<models::Payment> find_all() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<models::Payment> result;
    result.reserve(store_.size());
    for (auto &[id, p] : store_)
      result.push_back(p);
    return result;
  }

  std::size_t count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return store_.size();
  }

private:
  static void write_str(std::ofstream &f, const std::string &s) {
    std::size_t len = s.size();
    f.write(reinterpret_cast<const char *>(&len), sizeof(len));
    f.write(s.data(), static_cast<std::streamsize>(len));
  }
  static void read_str(std::ifstream &f, std::string &s) {
    std::size_t len = 0;
    f.read(reinterpret_cast<char *>(&len), sizeof(len));
    s.resize(len);
    f.read(s.data(), static_cast<std::streamsize>(len));
  }

  void write_payment(std::ofstream &f, const models::Payment &p) {
    f.write(reinterpret_cast<const char *>(&p.id), sizeof(p.id));
    f.write(reinterpret_cast<const char *>(&p.invoice_id),
            sizeof(p.invoice_id));
    f.write(reinterpret_cast<const char *>(&p.customer_id),
            sizeof(p.customer_id));
    f.write(reinterpret_cast<const char *>(&p.method), sizeof(p.method));
    f.write(reinterpret_cast<const char *>(&p.status), sizeof(p.status));
    f.write(reinterpret_cast<const char *>(&p.amount), sizeof(p.amount));
    f.write(reinterpret_cast<const char *>(&p.refund_amount),
            sizeof(p.refund_amount));
    write_str(f, p.gateway_ref);
    write_str(f, p.currency);
    write_str(f, p.notes);
    f.write(reinterpret_cast<const char *>(&p.retry_count),
            sizeof(p.retry_count));
    f.write(reinterpret_cast<const char *>(&p.fraud_flagged),
            sizeof(p.fraud_flagged));
    f.write(reinterpret_cast<const char *>(&p.created_at),
            sizeof(p.created_at));
    f.write(reinterpret_cast<const char *>(&p.completed_at),
            sizeof(p.completed_at));
  }

  void read_payment(std::ifstream &f, models::Payment &p) {
    f.read(reinterpret_cast<char *>(&p.id), sizeof(p.id));
    f.read(reinterpret_cast<char *>(&p.invoice_id), sizeof(p.invoice_id));
    f.read(reinterpret_cast<char *>(&p.customer_id), sizeof(p.customer_id));
    f.read(reinterpret_cast<char *>(&p.method), sizeof(p.method));
    f.read(reinterpret_cast<char *>(&p.status), sizeof(p.status));
    f.read(reinterpret_cast<char *>(&p.amount), sizeof(p.amount));
    f.read(reinterpret_cast<char *>(&p.refund_amount), sizeof(p.refund_amount));
    read_str(f, p.gateway_ref);
    read_str(f, p.currency);
    read_str(f, p.notes);
    f.read(reinterpret_cast<char *>(&p.retry_count), sizeof(p.retry_count));
    f.read(reinterpret_cast<char *>(&p.fraud_flagged), sizeof(p.fraud_flagged));
    f.read(reinterpret_cast<char *>(&p.created_at), sizeof(p.created_at));
    f.read(reinterpret_cast<char *>(&p.completed_at), sizeof(p.completed_at));
  }

  void load_all() {
    std::ifstream f(data_file_, std::ios::binary);
    if (!f.is_open())
      return;
    std::size_t count = 0;
    f.read(reinterpret_cast<char *>(&count), sizeof(count));
    for (std::size_t i = 0; i < count; ++i) {
      models::Payment p;
      read_payment(f, p);
      store_[p.id] = p;
    }
  }

  void flush() {
    std::ofstream f(data_file_, std::ios::binary | std::ios::trunc);
    if (!f.is_open())
      throw std::runtime_error("Cannot open payment data file for writing");
    std::size_t count = store_.size();
    f.write(reinterpret_cast<const char *>(&count), sizeof(count));
    for (auto &[id, p] : store_)
      write_payment(f, p);
  }

  std::string data_file_;
  std::unordered_map<int64_t, models::Payment> store_;
  mutable core::LRUCache<int64_t, models::Payment> cache_;
  mutable std::mutex mutex_;
};

} // namespace billing::repository
