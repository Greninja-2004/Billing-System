#pragma once
// =============================================================================
// min_heap.hpp — Min-Heap Priority Queue for Payment Scheduling
// Used for: Ordering invoices/payments by due date (earliest first)
// Complexity: Push O(log n), Pop O(log n), Peek O(1)
// =============================================================================
#include <algorithm>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <vector>

namespace billing::core {

template <typename T, typename Compare = std::less<T>> class MinHeap {
public:
  explicit MinHeap(Compare comp = Compare()) : comp_(comp) {}

  // Push element — O(log n)
  void push(T value) {
    std::lock_guard<std::mutex> lock(mutex_);
    data_.push_back(std::move(value));
    sift_up(data_.size() - 1);
  }

  // Remove and return smallest element — O(log n)
  T pop() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (data_.empty())
      throw std::underflow_error("Heap is empty");
    T top = std::move(data_[0]);
    data_[0] = std::move(data_.back());
    data_.pop_back();
    if (!data_.empty())
      sift_down(0);
    return top;
  }

  // Peek at smallest without removing — O(1)
  const T &peek() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (data_.empty())
      throw std::underflow_error("Heap is empty");
    return data_[0];
  }

  bool empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return data_.empty();
  }

  std::size_t size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return data_.size();
  }

  // Build heap from vector — O(n)
  void build(std::vector<T> items) {
    std::lock_guard<std::mutex> lock(mutex_);
    data_ = std::move(items);
    for (int i = static_cast<int>(data_.size() / 2) - 1; i >= 0; --i)
      sift_down(i);
  }

  // Drain all elements in sorted order
  std::vector<T> drain_sorted() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<T> result;
    result.reserve(data_.size());
    // Copy heap and sort
    std::vector<T> temp = data_;
    while (!temp.empty()) {
      result.push_back(temp.front());
      std::swap(temp.front(), temp.back());
      temp.pop_back();
      // sift down without mutex (already locked)
      std::size_t i = 0;
      while (true) {
        std::size_t l = 2 * i + 1, r = 2 * i + 2, best = i;
        if (l < temp.size() && comp_(temp[l], temp[best]))
          best = l;
        if (r < temp.size() && comp_(temp[r], temp[best]))
          best = r;
        if (best == i)
          break;
        std::swap(temp[i], temp[best]);
        i = best;
      }
    }
    return result;
  }

  void clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    data_.clear();
  }

private:
  void sift_up(std::size_t i) {
    while (i > 0) {
      std::size_t parent = (i - 1) / 2;
      if (comp_(data_[i], data_[parent])) {
        std::swap(data_[i], data_[parent]);
        i = parent;
      } else
        break;
    }
  }

  void sift_down(std::size_t i) {
    std::size_t n = data_.size();
    while (true) {
      std::size_t l = 2 * i + 1, r = 2 * i + 2, best = i;
      if (l < n && comp_(data_[l], data_[best]))
        best = l;
      if (r < n && comp_(data_[r], data_[best]))
        best = r;
      if (best == i)
        break;
      std::swap(data_[i], data_[best]);
      i = best;
    }
  }

  std::vector<T> data_;
  Compare comp_;
  mutable std::mutex mutex_;
};

} // namespace billing::core
