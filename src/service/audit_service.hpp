#pragma once
// =============================================================================
// audit_service.hpp — Immutable Append-Only Audit Trail
// Design Pattern: Singleton
// Complexity: Append O(1), Read O(n)
// =============================================================================
#include "../models/audit_log.hpp"
#include <atomic>
#include <ctime>
#include <fstream>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace billing::service {

class AuditService {
public:
  // Singleton pattern
  static AuditService &instance(const std::string &log_file = "") {
    static AuditService inst(log_file.empty() ? "data/audit.log" : log_file);
    return inst;
  }

  // Append an audit entry (never modifies existing) — O(1)
  void log(const std::string &user_id, models::AuditAction action,
           const std::string &entity_type, int64_t entity_id,
           const std::string &description,
           const std::string &ip = "127.0.0.1") {
    std::lock_guard<std::mutex> lock(mutex_);
    models::AuditLog entry;
    entry.sequence = ++sequence_;
    entry.timestamp = std::time(nullptr);
    entry.user_id = user_id;
    entry.action = action;
    entry.entity_type = entity_type;
    entry.entity_id = entity_id;
    entry.description = description;
    entry.ip_address = ip;
    entry.checksum = entry.compute_checksum();

    log_cache_.push_back(entry);
    append_to_file(entry);
  }

  // Read all audit entries — O(n)
  std::vector<models::AuditLog> read_all() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return log_cache_;
  }

  // Verify integrity of all entries — O(n)
  bool verify_integrity() const {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &entry : log_cache_)
      if (!entry.verify())
        return false;
    return true;
  }

  // Filter by entity type — O(n)
  std::vector<models::AuditLog>
  filter_by_entity(const std::string &entity_type) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<models::AuditLog> result;
    for (auto &e : log_cache_)
      if (e.entity_type == entity_type)
        result.push_back(e);
    return result;
  }

  // Filter by user — O(n)
  std::vector<models::AuditLog>
  filter_by_user(const std::string &user_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<models::AuditLog> result;
    for (auto &e : log_cache_)
      if (e.user_id == user_id)
        result.push_back(e);
    return result;
  }

  std::size_t count() const { return log_cache_.size(); }

private:
  explicit AuditService(const std::string &log_file)
      : log_file_(log_file), sequence_(0) {
    load_from_file();
  }

  AuditService(const AuditService &) = delete;
  AuditService &operator=(const AuditService &) = delete;

  void append_to_file(const models::AuditLog &e) {
    std::ofstream f(log_file_, std::ios::app);
    if (!f.is_open())
      return; // silently fail — audit shouldn't crash system

    std::time_t ts = e.timestamp;
    std::tm *t = std::localtime(&ts);
    char timebuf[32];
    std::strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", t);

    f << e.sequence << "|" << timebuf << "|" << e.user_id << "|"
      << models::audit_action_to_string(e.action) << "|" << e.entity_type << "|"
      << e.entity_id << "|" << e.description << "|" << e.ip_address << "|"
      << std::hex << e.checksum << "\n";
  }

  void load_from_file() {
    std::ifstream f(log_file_);
    if (!f.is_open())
      return;
    std::string line;
    while (std::getline(f, line)) {
      if (line.empty())
        continue;
      // Parse pipe-delimited format
      // sequence|timestamp|user_id|action|entity_type|entity_id|description|ip|checksum
      models::AuditLog e;
      std::istringstream ss(line);
      std::string token;
      std::vector<std::string> parts;
      while (std::getline(ss, token, '|'))
        parts.push_back(token);
      if (parts.size() < 9)
        continue;
      e.sequence = std::stoll(parts[0]);
      // timestamp: parse from string
      e.timestamp = std::time(nullptr); // simplified
      e.user_id = parts[2];
      e.entity_type = parts[4];
      e.entity_id = std::stoll(parts[5]);
      e.description = parts[6];
      e.ip_address = parts[7];
      e.checksum = static_cast<uint32_t>(std::stoul(parts[8], nullptr, 16));
      log_cache_.push_back(e);
      if (e.sequence > sequence_)
        sequence_ = e.sequence;
    }
  }

  std::string log_file_;
  std::vector<models::AuditLog> log_cache_;
  mutable std::mutex mutex_;
  std::atomic<int64_t> sequence_;
};

// Convenience macro
#define AUDIT(user, action, entity, id, desc)                                  \
  billing::service::AuditService::instance().log(user, action, entity, id, desc)

} // namespace billing::service
