#pragma once
// =============================================================================
// customer_service.hpp — Customer Management Service
// Design Patterns: Factory (for Customer creation)
// =============================================================================
#include "../core/snowflake.hpp"
#include "../models/customer.hpp"
#include "../repository/customer_repository.hpp"
#include <ctime>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace billing::service {

struct CustomerCreateRequest {
  std::string name;
  std::string email;
  std::string phone;
  std::string address;
  std::string country;
  std::string state;
};

class CustomerService {
public:
  explicit CustomerService(repository::CustomerRepository &repo)
      : repo_(repo) {}

  // Factory method — creates a new customer
  models::Customer create(const CustomerCreateRequest &req) {
    if (req.name.empty() || req.email.empty())
      throw std::invalid_argument("Name and email are required");
    if (repo_.find_by_email(req.email))
      throw std::runtime_error("Email already registered: " + req.email);

    models::Customer c;
    c.id = core::generate_id();
    c.name = req.name;
    c.email = req.email;
    c.phone = req.phone;
    c.address = req.address;
    c.country = req.country;
    c.state = req.state;
    c.tier = models::CustomerTier::BRONZE;
    c.status = models::CustomerStatus::ACTIVE;
    c.credit_score = 650; // default neutral score
    c.credit_limit = compute_credit_limit(c.credit_score, c.tier);
    c.current_balance = 0.0;
    c.total_spent = 0.0;
    c.created_at = std::time(nullptr);
    c.updated_at = c.created_at;

    repo_.save(c);
    return c;
  }

  std::optional<models::Customer> get(int64_t id) {
    return repo_.find_by_id(id);
  }

  std::optional<models::Customer> get_by_email(const std::string &email) {
    return repo_.find_by_email(email);
  }

  std::vector<models::Customer> list_all() { return repo_.find_all(); }

  std::vector<models::Customer> list_by_tier(models::CustomerTier tier) {
    return repo_.find_by_tier(tier);
  }

  // Update basic profile fields
  bool update_profile(int64_t id, const std::string &name,
                      const std::string &phone, const std::string &address) {
    auto opt = repo_.find_by_id(id);
    if (!opt)
      return false;
    auto c = *opt;
    c.name = name;
    c.phone = phone;
    c.address = address;
    c.updated_at = std::time(nullptr);
    return repo_.update(c);
  }

  // Recalculate credit score and auto-adjust tier + limit
  bool recalculate_credit(int64_t id, double payment_amount, bool on_time) {
    auto opt = repo_.find_by_id(id);
    if (!opt)
      return false;
    auto c = *opt;

    // Weighted credit score adjustment
    int delta = on_time ? +5 : -15;
    if (payment_amount > 1000)
      delta += on_time ? +3 : -5;
    c.credit_score = std::max(300, std::min(850, c.credit_score + delta));
    c.total_spent += payment_amount;

    // Auto-tier upgrade/downgrade
    c.tier = models::Customer::compute_tier(c.total_spent);
    c.credit_limit = compute_credit_limit(c.credit_score, c.tier);
    c.updated_at = std::time(nullptr);
    return repo_.update(c);
  }

  bool suspend(int64_t id) {
    auto opt = repo_.find_by_id(id);
    if (!opt)
      return false;
    auto c = *opt;
    c.status = models::CustomerStatus::SUSPENDED;
    c.updated_at = std::time(nullptr);
    return repo_.update(c);
  }

  bool activate(int64_t id) {
    auto opt = repo_.find_by_id(id);
    if (!opt)
      return false;
    auto c = *opt;
    c.status = models::CustomerStatus::ACTIVE;
    c.updated_at = std::time(nullptr);
    return repo_.update(c);
  }

  bool remove(int64_t id) { return repo_.remove(id); }

  std::size_t count() { return repo_.count(); }

  // Dynamic credit limit based on score + tier
  static double compute_credit_limit(int score, models::CustomerTier tier) {
    double base = 0;
    switch (tier) {
    case models::CustomerTier::BRONZE:
      base = 1000.0;
      break;
    case models::CustomerTier::SILVER:
      base = 5000.0;
      break;
    case models::CustomerTier::GOLD:
      base = 25000.0;
      break;
    case models::CustomerTier::ENTERPRISE:
      base = 100000.0;
      break;
    }
    // Scale by credit score ratio (300–850 range)
    double ratio = static_cast<double>(score - 300) / 550.0;
    return base * (0.5 + ratio);
  }

private:
  repository::CustomerRepository &repo_;
};

} // namespace billing::service
