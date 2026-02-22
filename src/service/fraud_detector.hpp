#pragma once
// =============================================================================
// fraud_detector.hpp — Sliding Window Frequency Analysis Fraud Detection
// Complexity: O(1) amortized per transaction (circular buffer)
// =============================================================================
#include <chrono>
#include <deque>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace billing::service {

struct FraudSignal {
  bool flagged;
  std::string reason;
  double risk_score; // 0.0 – 1.0
};

class FraudDetector {
public:
  // Window = 60 seconds, max 10 transactions, amount threshold $5000
  explicit FraudDetector(int window_sec = 60, int max_tx = 10,
                         double amount_threshold = 5000.0)
      : window_sec_(window_sec), max_tx_(max_tx),
        amount_threshold_(amount_threshold) {}

  // Record a transaction and return fraud signal — O(1) amortized
  FraudSignal check(int64_t customer_id, double amount) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto now = current_epoch_sec();
    auto &window = windows_[customer_id];

    // Evict expired entries from front
    while (!window.empty() && window.front().timestamp < now - window_sec_)
      window.pop_front();

    // Record this transaction
    window.push_back({now, amount});

    FraudSignal sig{false, "", 0.0};

    // Rule 1: High frequency — too many transactions in window
    if (static_cast<int>(window.size()) > max_tx_) {
      sig.flagged = true;
      sig.reason += "High frequency: " + std::to_string(window.size()) +
                    " transactions in " + std::to_string(window_sec_) + "s. ";
      sig.risk_score += 0.5;
    }

    // Rule 2: High amount transaction
    if (amount > amount_threshold_) {
      sig.reason += "Large amount: $" + std::to_string(amount) + ". ";
      sig.risk_score += 0.3;
      if (amount > amount_threshold_ * 3) {
        sig.flagged = true;
        sig.risk_score += 0.2;
      }
    }

    // Rule 3: Multiple large transactions in window
    int large_tx = 0;
    double window_total = 0.0;
    for (auto &e : window) {
      window_total += e.amount;
      if (e.amount > amount_threshold_)
        large_tx++;
    }
    if (large_tx >= 3) {
      sig.flagged = true;
      sig.reason +=
          "Multiple large transactions: " + std::to_string(large_tx) + ". ";
      sig.risk_score += 0.4;
    }

    // Rule 4: Window total exceeds 5x threshold
    if (window_total > amount_threshold_ * 5) {
      sig.flagged = true;
      sig.reason +=
          "Window total $" + std::to_string(window_total) + " exceeds limit. ";
      sig.risk_score += 0.3;
    }

    sig.risk_score = std::min(1.0, sig.risk_score);
    return sig;
  }

  // Get transaction count for customer in current window
  int transaction_count(int64_t customer_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto now = current_epoch_sec();
    auto &window = windows_[customer_id];
    while (!window.empty() && window.front().timestamp < now - window_sec_)
      window.pop_front();
    return static_cast<int>(window.size());
  }

  void clear_customer(int64_t customer_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    windows_.erase(customer_id);
  }

  void update_params(int window_sec, int max_tx, double threshold) {
    std::lock_guard<std::mutex> lock(mutex_);
    window_sec_ = window_sec;
    max_tx_ = max_tx;
    amount_threshold_ = threshold;
  }

private:
  struct TxEntry {
    int64_t timestamp;
    double amount;
  };

  static int64_t current_epoch_sec() {
    using namespace std::chrono;
    return duration_cast<seconds>(system_clock::now().time_since_epoch())
        .count();
  }

  int window_sec_;
  int max_tx_;
  double amount_threshold_;
  std::unordered_map<int64_t, std::deque<TxEntry>> windows_;
  std::mutex mutex_;
};

} // namespace billing::service
