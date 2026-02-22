#pragma once
// =============================================================================
// lru_cache.hpp — LRU Cache using Doubly-Linked List + Hash Map
// Used for: Caching frequently accessed billing records
// Complexity: Get O(1), Put O(1), Evict O(1)
// =============================================================================
#include <functional>
#include <list>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <unordered_map>

namespace billing::core {

template <typename Key, typename Value> class LRUCache {
  using ListIter = typename std::list<std::pair<Key, Value>>::iterator;

public:
  explicit LRUCache(std::size_t capacity) : capacity_(capacity) {
    if (capacity == 0)
      throw std::invalid_argument("LRU capacity must be > 0");
  }

  // Get value by key, moves to front — O(1)
  std::optional<Value> get(const Key &key) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = map_.find(key);
    if (it == map_.end()) {
      misses_++;
      return std::nullopt;
    }
    hits_++;
    // Move to front (most recently used)
    list_.splice(list_.begin(), list_, it->second);
    return it->second->second;
  }

  // Put key-value, evict LRU if full — O(1)
  void put(const Key &key, const Value &value) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = map_.find(key);
    if (it != map_.end()) {
      it->second->second = value;
      list_.splice(list_.begin(), list_, it->second);
      return;
    }
    if (list_.size() == capacity_) {
      // Evict LRU (back of list)
      auto &lru = list_.back();
      if (evict_cb_)
        evict_cb_(lru.first, lru.second);
      map_.erase(lru.first);
      list_.pop_back();
    }
    list_.emplace_front(key, value);
    map_[key] = list_.begin();
  }

  // Invalidate a specific key — O(1)
  bool evict(const Key &key) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = map_.find(key);
    if (it == map_.end())
      return false;
    list_.erase(it->second);
    map_.erase(it);
    return true;
  }

  bool contains(const Key &key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return map_.count(key) > 0;
  }

  // Set eviction callback (e.g., flush to disk)
  void set_evict_callback(std::function<void(const Key &, const Value &)> cb) {
    evict_cb_ = std::move(cb);
  }

  std::size_t size() const { return list_.size(); }
  std::size_t capacity() const { return capacity_; }
  std::size_t hits() const { return hits_; }
  std::size_t misses() const { return misses_; }
  double hit_rate() const {
    std::size_t total = hits_ + misses_;
    return total ? static_cast<double>(hits_) / total : 0.0;
  }

  void clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    list_.clear();
    map_.clear();
  }

private:
  std::size_t capacity_;
  std::list<std::pair<Key, Value>> list_;
  std::unordered_map<Key, ListIter> map_;
  std::function<void(const Key &, const Value &)> evict_cb_;
  mutable std::mutex mutex_;
  std::size_t hits_ = 0;
  std::size_t misses_ = 0;
};

} // namespace billing::core
