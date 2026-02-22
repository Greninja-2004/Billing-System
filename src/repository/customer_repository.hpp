#pragma once
// =============================================================================
// customer_repository.hpp — File-based Customer Persistence
// Binary serialization with B+ Tree indexing + LRU Cache
// =============================================================================
#include "../core/bplus_tree.hpp"
#include "../core/lru_cache.hpp"
#include "../models/customer.hpp"
#include <fstream>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace billing::repository {

class CustomerRepository {
public:
  explicit CustomerRepository(const std::string &data_dir)
      : data_file_(data_dir + "/customers.bin"), index_(/* B+ Tree order 4 */),
        cache_(256) {
    load_all();
  }

  // Create — O(log n) index insert
  void save(const models::Customer &customer) {
    std::lock_guard<std::mutex> lock(mutex_);
    store_[customer.id] = customer;
    index_.insert(customer.id, customer.id);
    cache_.put(customer.id, customer);
    flush();
  }

  // Read by ID — O(1) cache, O(log n) fallback
  std::optional<models::Customer> find_by_id(int64_t id) {
    // Try cache first
    auto cached = cache_.get(id);
    if (cached)
      return cached;
    // Fallback to index + store
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = store_.find(id);
    if (it == store_.end())
      return std::nullopt;
    cache_.put(id, it->second);
    return it->second;
  }

  // Find by email — O(n) linear scan (in production: secondary index)
  std::optional<models::Customer> find_by_email(const std::string &email) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &[id, c] : store_)
      if (c.email == email)
        return c;
    return std::nullopt;
  }

  // Update — O(log n)
  bool update(const models::Customer &customer) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (store_.find(customer.id) == store_.end())
      return false;
    store_[customer.id] = customer;
    index_.update(customer.id, customer.id);
    cache_.put(customer.id, customer);
    flush();
    return true;
  }

  // Delete — O(log n)
  bool remove(int64_t id) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (store_.erase(id) == 0)
      return false;
    index_.remove(id);
    cache_.evict(id);
    flush();
    return true;
  }

  // Get all customers — O(n)
  std::vector<models::Customer> find_all() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<models::Customer> result;
    result.reserve(store_.size());
    for (auto &[id, c] : store_)
      result.push_back(c);
    return result;
  }

  // Range query by ID range using B+ Tree — O(log n + k)
  std::vector<models::Customer> find_range(int64_t lo, int64_t hi) const {
    auto ids = index_.range(lo, hi);
    std::vector<models::Customer> result;
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &[id, _] : ids) {
      auto it = store_.find(id);
      if (it != store_.end())
        result.push_back(it->second);
    }
    return result;
  }

  // Find by tier — O(n)
  std::vector<models::Customer> find_by_tier(models::CustomerTier tier) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<models::Customer> result;
    for (auto &[id, c] : store_)
      if (c.tier == tier)
        result.push_back(c);
    return result;
  }

  std::size_t count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return store_.size();
  }

  double cache_hit_rate() const { return cache_.hit_rate(); }

private:
  void load_all() {
    std::ifstream f(data_file_, std::ios::binary);
    if (!f.is_open())
      return; // file doesn't exist yet
    std::size_t count = 0;
    f.read(reinterpret_cast<char *>(&count), sizeof(count));
    for (std::size_t i = 0; i < count; ++i) {
      models::Customer c;
      read_customer(f, c);
      store_[c.id] = c;
      index_.insert(c.id, c.id);
    }
  }

  void flush() {
    std::ofstream f(data_file_, std::ios::binary | std::ios::trunc);
    if (!f.is_open())
      throw std::runtime_error("Cannot open customer data file for writing");
    std::size_t count = store_.size();
    f.write(reinterpret_cast<const char *>(&count), sizeof(count));
    for (auto &[id, c] : store_)
      write_customer(f, c);
  }

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

  void write_customer(std::ofstream &f, const models::Customer &c) {
    f.write(reinterpret_cast<const char *>(&c.id), sizeof(c.id));
    write_string(f, c.name);
    write_string(f, c.email);
    write_string(f, c.phone);
    write_string(f, c.address);
    write_string(f, c.country);
    write_string(f, c.state);
    f.write(reinterpret_cast<const char *>(&c.tier), sizeof(c.tier));
    f.write(reinterpret_cast<const char *>(&c.status), sizeof(c.status));
    f.write(reinterpret_cast<const char *>(&c.credit_score),
            sizeof(c.credit_score));
    f.write(reinterpret_cast<const char *>(&c.credit_limit),
            sizeof(c.credit_limit));
    f.write(reinterpret_cast<const char *>(&c.current_balance),
            sizeof(c.current_balance));
    f.write(reinterpret_cast<const char *>(&c.total_spent),
            sizeof(c.total_spent));
    f.write(reinterpret_cast<const char *>(&c.created_at),
            sizeof(c.created_at));
    f.write(reinterpret_cast<const char *>(&c.updated_at),
            sizeof(c.updated_at));
  }

  void read_customer(std::ifstream &f, models::Customer &c) {
    f.read(reinterpret_cast<char *>(&c.id), sizeof(c.id));
    read_string(f, c.name);
    read_string(f, c.email);
    read_string(f, c.phone);
    read_string(f, c.address);
    read_string(f, c.country);
    read_string(f, c.state);
    f.read(reinterpret_cast<char *>(&c.tier), sizeof(c.tier));
    f.read(reinterpret_cast<char *>(&c.status), sizeof(c.status));
    f.read(reinterpret_cast<char *>(&c.credit_score), sizeof(c.credit_score));
    f.read(reinterpret_cast<char *>(&c.credit_limit), sizeof(c.credit_limit));
    f.read(reinterpret_cast<char *>(&c.current_balance),
           sizeof(c.current_balance));
    f.read(reinterpret_cast<char *>(&c.total_spent), sizeof(c.total_spent));
    f.read(reinterpret_cast<char *>(&c.created_at), sizeof(c.created_at));
    f.read(reinterpret_cast<char *>(&c.updated_at), sizeof(c.updated_at));
  }

  std::string data_file_;
  std::unordered_map<int64_t, models::Customer> store_;
  core::BPlusTree<int64_t, int64_t> index_;
  mutable core::LRUCache<int64_t, models::Customer> cache_;
  mutable std::mutex mutex_;
};

} // namespace billing::repository
