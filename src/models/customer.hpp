#pragma once
// =============================================================================
// customer.hpp — Customer Profile Model
// =============================================================================
#include <cstdint>
#include <ctime>
#include <string>

namespace billing::models {

enum class CustomerTier { BRONZE = 0, SILVER = 1, GOLD = 2, ENTERPRISE = 3 };

enum class CustomerStatus { ACTIVE = 0, SUSPENDED = 1, CLOSED = 2 };

inline std::string tier_to_string(CustomerTier t) {
  switch (t) {
  case CustomerTier::BRONZE:
    return "Bronze";
  case CustomerTier::SILVER:
    return "Silver";
  case CustomerTier::GOLD:
    return "Gold";
  case CustomerTier::ENTERPRISE:
    return "Enterprise";
  }
  return "Unknown";
}

inline std::string status_to_string(CustomerStatus s) {
  switch (s) {
  case CustomerStatus::ACTIVE:
    return "Active";
  case CustomerStatus::SUSPENDED:
    return "Suspended";
  case CustomerStatus::CLOSED:
    return "Closed";
  }
  return "Unknown";
}

struct Customer {
  int64_t id;
  std::string name;
  std::string email;
  std::string phone;
  std::string address;
  std::string country;
  std::string state; // for tax jurisdiction
  CustomerTier tier;
  CustomerStatus status;
  int credit_score; // 300–850
  double credit_limit;
  double current_balance;
  double total_spent; // lifetime
  std::time_t created_at;
  std::time_t updated_at;

  // Computed
  double lifetime_months() const {
    return static_cast<double>(std::difftime(std::time(nullptr), created_at)) /
           (30.0 * 86400.0);
  }

  // Tier thresholds (total_spent based)
  static CustomerTier compute_tier(double total_spent) {
    if (total_spent >= 50000.0)
      return CustomerTier::ENTERPRISE;
    if (total_spent >= 10000.0)
      return CustomerTier::GOLD;
    if (total_spent >= 2000.0)
      return CustomerTier::SILVER;
    return CustomerTier::BRONZE;
  }
};

} // namespace billing::models
