#pragma once
// =============================================================================
// notification.hpp — Notification & Alert Model
// =============================================================================
#include <cstdint>
#include <ctime>
#include <string>

namespace billing::models {

enum class NotificationChannel { EMAIL = 0, SMS = 1, IN_APP = 2 };

enum class NotificationPriority {
  LOW = 3,
  MEDIUM = 2,
  HIGH = 1,
  CRITICAL = 0 // lowest = highest priority in min-heap
};

enum class NotificationStatus { QUEUED = 0, SENT = 1, FAILED = 2, READ = 3 };

enum class EscalationState {
  ACTIVE = 0,
  WARNED = 1,
  ESCALATED = 2,
  SUSPENDED = 3,
  CLOSED = 4
};

inline std::string escalation_state_to_string(EscalationState s) {
  switch (s) {
  case EscalationState::ACTIVE:
    return "Active";
  case EscalationState::WARNED:
    return "Warned";
  case EscalationState::ESCALATED:
    return "Escalated";
  case EscalationState::SUSPENDED:
    return "Suspended";
  case EscalationState::CLOSED:
    return "Closed";
  }
  return "Unknown";
}

struct Notification {
  int64_t id;
  int64_t customer_id;
  int64_t invoice_id; // 0 if not invoice-related
  NotificationChannel channel;
  NotificationPriority priority;
  NotificationStatus status;
  std::string subject;
  std::string body;
  std::time_t created_at;
  std::time_t sent_at;

  // Comparator for priority queue (lower priority value = higher priority)
  bool operator>(const Notification &o) const {
    return static_cast<int>(priority) > static_cast<int>(o.priority);
  }
  bool operator<(const Notification &o) const {
    return static_cast<int>(priority) < static_cast<int>(o.priority);
  }
};

} // namespace billing::models
