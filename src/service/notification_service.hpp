#pragma once
// =============================================================================
// notification_service.hpp — Priority Queue + State Machine Escalation Engine
// Design Pattern: Observer (BillingObserver), State Machine
// Complexity: Enqueue O(log n), Dequeue O(log n)
// =============================================================================
#include "../core/snowflake.hpp"
#include "../models/invoice.hpp"
#include "../models/notification.hpp"
#include "../service/billing_engine.hpp"
#include <ctime>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

namespace billing::service {

// Priority comparator for notification min-heap (CRITICAL = 0 = highest
// priority)
struct NotificationCompare {
  bool operator()(const models::Notification &a,
                  const models::Notification &b) const {
    return static_cast<int>(a.priority) > static_cast<int>(b.priority);
  }
};

class NotificationService : public BillingObserver {
public:
  NotificationService() {}

  // Enqueue a notification — O(log n)
  void enqueue(const models::Notification &n) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    queue_.push(n);
  }

  // Dequeue and dispatch highest priority notification — O(log n)
  std::optional<models::Notification> dispatch_next() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (queue_.empty())
      return std::nullopt;
    auto n = queue_.top();
    queue_.pop();
    const_cast<models::Notification &>(n).status =
        models::NotificationStatus::SENT;
    const_cast<models::Notification &>(n).sent_at = std::time(nullptr);
    sent_log_.push_back(n);
    dispatch_channel(n);
    return n;
  }

  // Dispatch all queued notifications
  int dispatch_all() {
    int count = 0;
    while (!queue_.empty()) {
      dispatch_next();
      count++;
    }
    return count;
  }

  // =========================================================================
  // Escalation State Machine
  // Transitions:  ACTIVE → WARNED → ESCALATED → SUSPENDED → CLOSED
  // =========================================================================
  models::EscalationState escalate(int64_t customer_id,
                                   models::EscalationState current) {
    models::EscalationState next = current;
    switch (current) {
    case models::EscalationState::ACTIVE:
      next = models::EscalationState::WARNED;
      send_warning(customer_id);
      break;
    case models::EscalationState::WARNED:
      next = models::EscalationState::ESCALATED;
      send_escalation_notice(customer_id);
      break;
    case models::EscalationState::ESCALATED:
      next = models::EscalationState::SUSPENDED;
      send_suspension_notice(customer_id);
      break;
    case models::EscalationState::SUSPENDED:
      next = models::EscalationState::CLOSED;
      send_closure_notice(customer_id);
      break;
    case models::EscalationState::CLOSED:
      break; // terminal
    }
    escalation_states_[customer_id] = next;
    return next;
  }

  models::EscalationState get_escalation_state(int64_t customer_id) const {
    auto it = escalation_states_.find(customer_id);
    return it == escalation_states_.end() ? models::EscalationState::ACTIVE
                                          : it->second;
  }

  void reset_escalation(int64_t customer_id) {
    escalation_states_[customer_id] = models::EscalationState::ACTIVE;
  }

  std::size_t queue_size() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return queue_.size();
  }

  const std::vector<models::Notification> &sent_log() const {
    return sent_log_;
  }

  // =========================================================================
  // BillingObserver implementation
  // =========================================================================
  void on_invoice_created(const models::Invoice &inv) override {
    models::Notification n;
    n.id = core::generate_id();
    n.customer_id = inv.customer_id;
    n.invoice_id = inv.id;
    n.channel = models::NotificationChannel::EMAIL;
    n.priority = models::NotificationPriority::MEDIUM;
    n.status = models::NotificationStatus::QUEUED;
    n.subject = "New Invoice: " + inv.invoice_number;
    n.body = "Your invoice " + inv.invoice_number + " for $" +
             std::to_string(inv.total_amount) + " is ready.";
    n.created_at = std::time(nullptr);
    enqueue(n);
  }

  void on_invoice_paid(const models::Invoice &inv) override {
    models::Notification n;
    n.id = core::generate_id();
    n.customer_id = inv.customer_id;
    n.invoice_id = inv.id;
    n.channel = models::NotificationChannel::EMAIL;
    n.priority = models::NotificationPriority::LOW;
    n.status = models::NotificationStatus::QUEUED;
    n.subject = "Payment Received: " + inv.invoice_number;
    n.body = "Thank you! Invoice " + inv.invoice_number + " is now paid.";
    n.created_at = std::time(nullptr);
    enqueue(n);
  }

  void on_invoice_overdue(const models::Invoice &inv) override {
    models::Notification n;
    n.id = core::generate_id();
    n.customer_id = inv.customer_id;
    n.invoice_id = inv.id;
    n.channel = models::NotificationChannel::SMS;
    n.priority = models::NotificationPriority::HIGH;
    n.status = models::NotificationStatus::QUEUED;
    n.subject = "OVERDUE: " + inv.invoice_number;
    n.body = "Invoice " + inv.invoice_number + " is overdue! Amount due: $" +
             std::to_string(inv.amount_due());
    n.created_at = std::time(nullptr);
    enqueue(n);
    // Trigger escalation
    auto state = get_escalation_state(inv.customer_id);
    escalate(inv.customer_id, state);
  }

private:
  void dispatch_channel(const models::Notification &n) {
    // Simulation — in production: call SMS/email API
    std::string channel;
    switch (n.channel) {
    case models::NotificationChannel::EMAIL:
      channel = "[EMAIL]";
      break;
    case models::NotificationChannel::SMS:
      channel = "[SMS]";
      break;
    case models::NotificationChannel::IN_APP:
      channel = "[APP]";
      break;
    }
    // Silently dispatch (output suppressed in production)
    (void)channel;
  }

  void send_warning(int64_t cid) {
    models::Notification n;
    n.id = core::generate_id();
    n.customer_id = cid;
    n.invoice_id = 0;
    n.channel = models::NotificationChannel::EMAIL;
    n.priority = models::NotificationPriority::HIGH;
    n.status = models::NotificationStatus::QUEUED;
    n.subject = "Payment Warning";
    n.body = "Your account has overdue invoices. Please pay immediately.";
    n.created_at = std::time(nullptr);
    enqueue(n);
  }

  void send_escalation_notice(int64_t cid) {
    models::Notification n;
    n.id = core::generate_id();
    n.customer_id = cid;
    n.invoice_id = 0;
    n.channel = models::NotificationChannel::SMS;
    n.priority = models::NotificationPriority::CRITICAL;
    n.status = models::NotificationStatus::QUEUED;
    n.subject = "URGENT: Account Escalated";
    n.body = "Your account has been escalated to collections.";
    n.created_at = std::time(nullptr);
    enqueue(n);
  }

  void send_suspension_notice(int64_t cid) {
    models::Notification n;
    n.id = core::generate_id();
    n.customer_id = cid;
    n.invoice_id = 0;
    n.channel = models::NotificationChannel::IN_APP;
    n.priority = models::NotificationPriority::CRITICAL;
    n.status = models::NotificationStatus::QUEUED;
    n.subject = "Account Suspended";
    n.body = "Your account has been suspended due to non-payment.";
    n.created_at = std::time(nullptr);
    enqueue(n);
  }

  void send_closure_notice(int64_t cid) {
    models::Notification n;
    n.id = core::generate_id();
    n.customer_id = cid;
    n.invoice_id = 0;
    n.channel = models::NotificationChannel::EMAIL;
    n.priority = models::NotificationPriority::CRITICAL;
    n.status = models::NotificationStatus::QUEUED;
    n.subject = "Account Closed";
    n.body = "Your account has been permanently closed.";
    n.created_at = std::time(nullptr);
    enqueue(n);
  }

  std::priority_queue<models::Notification, std::vector<models::Notification>,
                      NotificationCompare>
      queue_;
  mutable std::mutex queue_mutex_;
  std::unordered_map<int64_t, models::EscalationState> escalation_states_;
  std::vector<models::Notification> sent_log_;
};

} // namespace billing::service
