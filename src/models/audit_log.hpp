#pragma once
// =============================================================================
// audit_log.hpp — Immutable Audit Trail Entry
// =============================================================================
#include <cstdint>
#include <ctime>
#include <string>

namespace billing::models {

enum class AuditAction {
  CREATE = 0,
  READ = 1,
  UPDATE = 2,
  DELETE = 3,
  LOGIN = 4,
  LOGOUT = 5,
  PAYMENT = 6,
  REFUND = 7,
  EXPORT = 8,
  CONFIG = 9,
  FRAUD_FLAG = 10
};

inline std::string audit_action_to_string(AuditAction a) {
  switch (a) {
  case AuditAction::CREATE:
    return "CREATE";
  case AuditAction::READ:
    return "READ";
  case AuditAction::UPDATE:
    return "UPDATE";
  case AuditAction::DELETE:
    return "DELETE";
  case AuditAction::LOGIN:
    return "LOGIN";
  case AuditAction::LOGOUT:
    return "LOGOUT";
  case AuditAction::PAYMENT:
    return "PAYMENT";
  case AuditAction::REFUND:
    return "REFUND";
  case AuditAction::EXPORT:
    return "EXPORT";
  case AuditAction::CONFIG:
    return "CONFIG";
  case AuditAction::FRAUD_FLAG:
    return "FRAUD_FLAG";
  }
  return "UNKNOWN";
}

struct AuditLog {
  int64_t sequence; // monotonically increasing
  std::time_t timestamp;
  std::string user_id; // who performed the action
  AuditAction action;
  std::string entity_type; // "Customer", "Invoice", "Payment", ...
  int64_t entity_id;
  std::string description;
  std::string ip_address;
  uint32_t checksum; // XOR-based integrity check

  // Compute checksum for integrity verification
  uint32_t compute_checksum() const {
    uint32_t cs = 0;
    cs ^= static_cast<uint32_t>(sequence);
    cs ^= static_cast<uint32_t>(timestamp);
    cs ^= static_cast<uint32_t>(entity_id);
    cs ^= static_cast<uint32_t>(action);
    for (char c : user_id)
      cs ^= static_cast<uint32_t>(c);
    for (char c : entity_type)
      cs ^= static_cast<uint32_t>(c);
    for (char c : description)
      cs ^= static_cast<uint32_t>(c);
    return cs;
  }

  bool verify() const { return checksum == compute_checksum(); }
};

} // namespace billing::models
