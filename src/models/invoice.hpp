#pragma once
// =============================================================================
// invoice.hpp — Invoice Model
// =============================================================================
#include <cstdint>
#include <ctime>
#include <string>
#include <vector>

namespace billing::models {

enum class InvoiceType { ONE_TIME = 0, RECURRING = 1, PRORATED = 2 };

enum class RecurringPeriod {
  NONE = 0,
  DAILY = 1,
  WEEKLY = 2,
  MONTHLY = 3,
  YEARLY = 4
};

enum class InvoiceStatus {
  DRAFT = 0,
  PENDING = 1,
  PARTIALLY_PAID = 2,
  PAID = 3,
  OVERDUE = 4,
  CANCELLED = 5,
  REFUNDED = 6
};

inline std::string invoice_status_to_string(InvoiceStatus s) {
  switch (s) {
  case InvoiceStatus::DRAFT:
    return "Draft";
  case InvoiceStatus::PENDING:
    return "Pending";
  case InvoiceStatus::PARTIALLY_PAID:
    return "Partially Paid";
  case InvoiceStatus::PAID:
    return "Paid";
  case InvoiceStatus::OVERDUE:
    return "Overdue";
  case InvoiceStatus::CANCELLED:
    return "Cancelled";
  case InvoiceStatus::REFUNDED:
    return "Refunded";
  }
  return "Unknown";
}

inline std::string invoice_type_to_string(InvoiceType t) {
  switch (t) {
  case InvoiceType::ONE_TIME:
    return "One-Time";
  case InvoiceType::RECURRING:
    return "Recurring";
  case InvoiceType::PRORATED:
    return "Prorated";
  }
  return "Unknown";
}

struct LineItem {
  std::string description;
  int quantity;
  double unit_price;
  double total() const { return quantity * unit_price; }
};

struct Invoice {
  int64_t id; // Snowflake ID
  int64_t customer_id;
  int64_t parent_invoice_id;  // for recurring chain, 0 = root
  std::string invoice_number; // human-readable e.g. INV-20240001
  InvoiceType type;
  RecurringPeriod period;
  InvoiceStatus status;

  std::vector<LineItem> line_items;
  double subtotal;
  double discount_amount;
  double tax_amount;
  double total_amount;
  double amount_paid;

  std::string currency;     // USD, EUR, etc.
  std::string jurisdiction; // tax jurisdiction code
  std::string notes;

  std::time_t issue_date;
  std::time_t due_date;
  std::time_t paid_date;
  std::time_t next_billing_date; // for recurring

  // Proration fields
  std::time_t period_start;
  std::time_t period_end;

  double amount_due() const { return total_amount - amount_paid; }
  bool is_overdue() const {
    return status != InvoiceStatus::PAID &&
           status != InvoiceStatus::CANCELLED && std::time(nullptr) > due_date;
  }
  int days_overdue() const {
    if (!is_overdue())
      return 0;
    return static_cast<int>(std::difftime(std::time(nullptr), due_date) /
                            86400.0);
  }
};

} // namespace billing::models
