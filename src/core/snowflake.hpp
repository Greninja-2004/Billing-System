#pragma once
// =============================================================================
// snowflake.hpp — 64-bit Snowflake ID Generator
// Used for: Globally unique invoice and transaction IDs
// Format: [timestamp 41 bits][worker 10 bits][sequence 12 bits]
// Complexity: O(1) per ID, thread-safe
// =============================================================================
#include <chrono>
#include <cstdint>
#include <mutex>
#include <stdexcept>

namespace billing::core {

class SnowflakeGenerator {
public:
  // Custom epoch: 2024-01-01 00:00:00 UTC (ms)
  static constexpr int64_t EPOCH = 1704067200000LL;
  static constexpr int64_t WORKER_BITS = 10;
  static constexpr int64_t SEQ_BITS = 12;
  static constexpr int64_t MAX_WORKER = (1LL << WORKER_BITS) - 1; // 1023
  static constexpr int64_t MAX_SEQ = (1LL << SEQ_BITS) - 1;       // 4095
  static constexpr int64_t WORKER_SHIFT = SEQ_BITS;
  static constexpr int64_t TS_SHIFT = WORKER_BITS + SEQ_BITS;

  explicit SnowflakeGenerator(int64_t worker_id = 1)
      : worker_id_(worker_id), sequence_(0), last_ts_(-1) {
    if (worker_id < 0 || worker_id > MAX_WORKER)
      throw std::invalid_argument("Worker ID out of range [0, 1023]");
  }

  // Generate next unique ID — O(1)
  int64_t next() {
    std::lock_guard<std::mutex> lock(mutex_);
    int64_t ts = current_ms();

    if (ts == last_ts_) {
      sequence_ = (sequence_ + 1) & MAX_SEQ;
      if (sequence_ == 0) {
        // Sequence exhausted, wait for next ms
        while ((ts = current_ms()) <= last_ts_) {
        }
      }
    } else {
      sequence_ = 0;
    }
    last_ts_ = ts;

    return ((ts - EPOCH) << TS_SHIFT) | (worker_id_ << WORKER_SHIFT) |
           sequence_;
  }

  // Decode components from ID
  struct DecodedID {
    int64_t timestamp_ms;
    int64_t worker_id;
    int64_t sequence;
  };

  static DecodedID decode(int64_t id) {
    return {(id >> TS_SHIFT) + EPOCH, (id >> WORKER_SHIFT) & MAX_WORKER,
            id & MAX_SEQ};
  }

  // Singleton accessor
  static SnowflakeGenerator &instance(int64_t worker_id = 1) {
    static SnowflakeGenerator inst(worker_id);
    return inst;
  }

private:
  int64_t current_ms() const {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch())
        .count();
  }

  int64_t worker_id_;
  int64_t sequence_;
  int64_t last_ts_;
  std::mutex mutex_;
};

// Convenience function
inline int64_t generate_id() { return SnowflakeGenerator::instance().next(); }

} // namespace billing::core
