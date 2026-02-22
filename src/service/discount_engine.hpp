#pragma once
// =============================================================================
// discount_engine.hpp — Rule-Based Discount Decision Tree
// Design Pattern: Strategy Pattern (DiscountStrategy interface)
// Complexity: O(log r) where r = number of rules (sorted priority)
// =============================================================================
#include "../models/customer.hpp"
#include "../models/invoice.hpp"
#include <algorithm>
#include <ctime>
#include <functional>
#include <string>
#include <vector>

namespace billing::service {

// ---------------------------------------------------------------------------
// Abstract Strategy Interface
// ---------------------------------------------------------------------------
struct DiscountStrategy {
  virtual ~DiscountStrategy() = default;
  virtual double compute(double subtotal, const models::Customer &customer,
                         const models::Invoice &invoice) const = 0;
  virtual std::string name() const = 0;
};

struct PercentageDiscount : DiscountStrategy {
  double rate; // 0.0–1.0
  std::string label;
  PercentageDiscount(double r, std::string l) : rate(r), label(std::move(l)) {}
  double compute(double subtotal, const models::Customer &,
                 const models::Invoice &) const override {
    return subtotal * rate;
  }
  std::string name() const override { return label; }
};

struct FlatDiscount : DiscountStrategy {
  double amount;
  std::string label;
  FlatDiscount(double a, std::string l) : amount(a), label(std::move(l)) {}
  double compute(double subtotal, const models::Customer &,
                 const models::Invoice &) const override {
    return std::min(amount, subtotal);
  }
  std::string name() const override { return label; }
};

struct TierDiscount : DiscountStrategy {
  std::string name() const override { return "Tier Discount"; }
  double compute(double subtotal, const models::Customer &c,
                 const models::Invoice &) const override {
    switch (c.tier) {
    case models::CustomerTier::BRONZE:
      return subtotal * 0.00;
    case models::CustomerTier::SILVER:
      return subtotal * 0.05;
    case models::CustomerTier::GOLD:
      return subtotal * 0.10;
    case models::CustomerTier::ENTERPRISE:
      return subtotal * 0.20;
    }
    return 0.0;
  }
};

// ---------------------------------------------------------------------------
// Discount Rule (node in decision tree)
// ---------------------------------------------------------------------------
struct DiscountRule {
  int priority; // lower = higher priority
  std::string condition_desc;
  std::function<bool(const models::Customer &, const models::Invoice &)>
      condition;
  std::shared_ptr<DiscountStrategy> strategy;
  bool combinable; // allow stacking with other rules
};

// ---------------------------------------------------------------------------
// DiscountEngine — evaluates rules and applies best discount
// ---------------------------------------------------------------------------
class DiscountEngine {
public:
  DiscountEngine() { load_default_rules(); }

  void add_rule(DiscountRule rule) {
    rules_.push_back(std::move(rule));
    sort_rules();
  }

  // Evaluate all rules and return total discount amount — O(r)
  double apply(double subtotal, const models::Customer &customer,
               const models::Invoice &invoice) const {
    double total_discount = 0.0;
    bool primary_applied = false;

    for (const auto &rule : rules_) {
      if (!rule.condition(customer, invoice))
        continue;

      if (!primary_applied || rule.combinable) {
        double d = rule.strategy->compute(subtotal, customer, invoice);
        total_discount += d;
        primary_applied = true;
      }
    }
    return std::min(total_discount, subtotal * 0.5); // cap at 50%
  }

  // Get applicable rule descriptions for an invoice
  std::vector<std::string> applicable_rules(const models::Customer &c,
                                            const models::Invoice &inv) const {
    std::vector<std::string> result;
    for (const auto &rule : rules_)
      if (rule.condition(c, inv))
        result.push_back(rule.condition_desc + " → " + rule.strategy->name());
    return result;
  }

private:
  void sort_rules() {
    std::sort(rules_.begin(), rules_.end(),
              [](const DiscountRule &a, const DiscountRule &b) {
                return a.priority < b.priority;
              });
  }

  void load_default_rules() {
    // Rule 1: Tier-based discount (always combinable)
    rules_.push_back(
        {1, "Tier loyalty discount",
         [](const models::Customer &, const models::Invoice &) { return true; },
         std::make_shared<TierDiscount>(), true});

    // Rule 2: Large invoice discount (>= $5000 subtotal)
    rules_.push_back(
        {2, "Large invoice (>=$5000) 5% off",
         [](const models::Customer &, const models::Invoice &inv) {
           return inv.subtotal >= 5000.0;
         },
         std::make_shared<PercentageDiscount>(0.05, "Volume Discount"), false});

    // Rule 3: Long-term customer (>= 12 months)
    rules_.push_back(
        {3, "Long-term customer (12m+) $50 off",
         [](const models::Customer &c, const models::Invoice &) {
           return c.lifetime_months() >= 12.0;
         },
         std::make_shared<FlatDiscount>(50.0, "Loyalty Flat Discount"), true});

    // Rule 4: Recurring invoice 3% off
    rules_.push_back(
        {4, "Recurring invoice 3% off",
         [](const models::Customer &, const models::Invoice &inv) {
           return inv.type == models::InvoiceType::RECURRING;
         },
         std::make_shared<PercentageDiscount>(0.03, "Recurring Discount"),
         true});

    // Rule 5: Enterprise flat $200 off
    rules_.push_back({5, "Enterprise flat $200 off",
                      [](const models::Customer &c, const models::Invoice &) {
                        return c.tier == models::CustomerTier::ENTERPRISE;
                      },
                      std::make_shared<FlatDiscount>(200.0, "Enterprise Bonus"),
                      true});

    sort_rules();
  }

  std::vector<DiscountRule> rules_;
};

} // namespace billing::service
