#pragma once
// =============================================================================
// payment.hpp — Payment & Transaction Model
// =============================================================================
#include <cstdint>
#include <ctime>
#include <string>
#include <vector>

namespace billing::models {

enum class PaymentMethod {
  CREDIT_CARD = 0,
  BANK_TRANSFER = 1,
  WALLET = 2,
  CASH = 3
};

enum class PaymentStatus {
  PENDING = 0,
  COMPLETED = 1,
  FAILED = 2,
  REFUNDED = 3,
  PARTIAL = 4,
  CANCELLED = 5
};

enum class GatewayResult {
  SUCCESS = 0,
  INSUFFICIENT_FUNDS = 1,
  CARD_DECLINED = 2,
  NETWORK_ERROR = 3,
  FRAUD_DETECTED = 4,
  TIMEOUT = 5
};

inline std::string payment_method_to_string(PaymentMethod m) {
  switch (m) {
  case PaymentMethod::CREDIT_CARD:
    return "Credit Card";
  case PaymentMethod::BANK_TRANSFER:
    return "Bank Transfer";
  case PaymentMethod::WALLET:
    return "Wallet";
  case PaymentMethod::CASH:
    return "Cash";
  }
  return "Unknown";
}

inline std::string payment_status_to_string(PaymentStatus s) {
  switch (s) {
  case PaymentStatus::PENDING:
    return "Pending";
  case PaymentStatus::COMPLETED:
    return "Completed";
  case PaymentStatus::FAILED:
    return "Failed";
  case PaymentStatus::REFUNDED:
    return "Refunded";
  case PaymentStatus::PARTIAL:
    return "Partial";
  case PaymentStatus::CANCELLED:
    return "Cancelled";
  }
  return "Unknown";
}

struct Payment {
  int64_t id; // Snowflake ID
  int64_t invoice_id;
  int64_t customer_id;
  PaymentMethod method;
  PaymentStatus status;
  double amount;
  double refund_amount;
  std::string gateway_ref; // external ref from gateway
  std::string currency;
  std::string notes;
  int retry_count;
  bool fraud_flagged;
  std::time_t created_at;
  std::time_t completed_at;
};

struct Refund {
  int64_t id;
  int64_t payment_id;
  int64_t invoice_id;
  double amount;
  std::string reason;
  std::time_t created_at;
};

} // namespace billing::models
