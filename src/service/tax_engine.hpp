#pragma once
// =============================================================================
// tax_engine.hpp — Jurisdiction-Based Tax Computation Engine
// Complexity: O(1) jurisdiction lookup via hash map, O(n) cascaded tax
// application
// =============================================================================
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace billing::service {

struct TaxRule {
  std::string jurisdiction_code; // e.g., "US-CA", "IN-MH", "UK"
  std::string description;
  double gst_rate;       // Goods & Services Tax
  double state_rate;     // State/regional tax
  double surcharge_rate; // Additional surcharge
  bool compound;         // If true, taxes compound (tax-on-tax)

  double total_rate() const { return gst_rate + state_rate + surcharge_rate; }
};

class TaxEngine {
public:
  TaxEngine() { load_default_rules(); }

  void add_rule(const TaxRule &rule) { rules_[rule.jurisdiction_code] = rule; }

  // Compute tax for a given subtotal and jurisdiction — O(1) lookup
  struct TaxResult {
    double gst_tax;
    double state_tax;
    double surcharge;
    double total_tax;
    std::string jurisdiction_code;
  };

  TaxResult compute(double subtotal,
                    const std::string &jurisdiction_code) const {
    auto it = rules_.find(jurisdiction_code);
    if (it == rules_.end()) {
      // Fallback: default 0% if jurisdiction not found
      return {0.0, 0.0, 0.0, 0.0, jurisdiction_code};
    }
    const TaxRule &rule = it->second;
    TaxResult result;
    result.jurisdiction_code = jurisdiction_code;

    if (rule.compound) {
      // Compound taxation: each tax applied on (subtotal + prev taxes)
      result.gst_tax = subtotal * rule.gst_rate;
      double after_gst = subtotal + result.gst_tax;
      result.state_tax = after_gst * rule.state_rate;
      double after_state = after_gst + result.state_tax;
      result.surcharge = after_state * rule.surcharge_rate;
    } else {
      // Simple cascaded: all taxes on original subtotal
      result.gst_tax = subtotal * rule.gst_rate;
      result.state_tax = subtotal * rule.state_rate;
      result.surcharge = subtotal * rule.surcharge_rate;
    }
    result.total_tax = result.gst_tax + result.state_tax + result.surcharge;
    return result;
  }

  // Get a human-readable tax breakdown string
  std::string format(const TaxResult &r) const {
    char buf[256];
    snprintf(buf, sizeof(buf),
             "GST: $%.2f | State: $%.2f | Surcharge: $%.2f | Total: $%.2f",
             r.gst_tax, r.state_tax, r.surcharge, r.total_tax);
    return std::string(buf);
  }

  // Derive jurisdiction code from country + state
  static std::string jurisdiction(const std::string &country,
                                  const std::string &state) {
    if (state.empty())
      return country;
    return country + "-" + state;
  }

  std::vector<std::string> available_jurisdictions() const {
    std::vector<std::string> j;
    for (auto &[k, _] : rules_)
      j.push_back(k);
    return j;
  }

private:
  void load_default_rules() {
    // United States
    rules_["US"] = {"US", "US Federal", 0.00, 0.00, 0.00, false};
    rules_["US-CA"] = {"US-CA", "California", 0.00, 0.0725, 0.01, false};
    rules_["US-NY"] = {"US-NY", "New York", 0.00, 0.08, 0.00, false};
    rules_["US-TX"] = {"US-TX", "Texas", 0.00, 0.0625, 0.02, false};
    rules_["US-FL"] = {"US-FL", "Florida", 0.00, 0.06, 0.00, false};
    rules_["US-WA"] = {"US-WA", "Washington", 0.00, 0.065, 0.00, false};
    // India
    rules_["IN"] = {"IN", "India GST", 0.18, 0.00, 0.00, false};
    rules_["IN-MH"] = {"IN-MH", "Maharashtra", 0.18, 0.00, 0.01, false};
    rules_["IN-KA"] = {"IN-KA", "Karnataka", 0.18, 0.00, 0.00, false};
    rules_["IN-DL"] = {"IN-DL", "Delhi", 0.18, 0.00, 0.005, false};
    // Europe
    rules_["UK"] = {"UK", "UK VAT", 0.20, 0.00, 0.00, false};
    rules_["EU"] = {"EU", "EU VAT", 0.21, 0.00, 0.00, false};
    rules_["DE"] = {"DE", "Germany VAT", 0.19, 0.00, 0.00, false};
    rules_["FR"] = {"FR", "France VAT", 0.20, 0.00, 0.00, false};
    // Zero tax
    rules_["SG"] = {"SG", "Singapore GST", 0.09, 0.00, 0.00, false};
    rules_["AE"] = {"AE", "UAE VAT", 0.05, 0.00, 0.00, false};
    rules_["HK"] = {"HK", "Hong Kong (0%)", 0.00, 0.00, 0.00, false};
  }

  std::unordered_map<std::string, TaxRule> rules_;
};

} // namespace billing::service
