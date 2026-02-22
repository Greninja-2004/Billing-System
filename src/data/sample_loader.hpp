#pragma once
// =============================================================================
// sample_loader.hpp — Sample Dataset Loader
// Generates 100+ customers and 500+ invoices for demonstration
// =============================================================================
#include "../models/customer.hpp"
#include "../models/invoice.hpp"
#include "../repository/customer_repository.hpp"
#include "../repository/invoice_repository.hpp"
#include "../service/billing_engine.hpp"
#include "../service/customer_service.hpp"
#include <ctime>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace billing::data {

class SampleLoader {
public:
  SampleLoader(service::CustomerService &cust_svc,
               service::BillingEngine &billing_eng)
      : cust_svc_(cust_svc), billing_eng_(billing_eng),
        rng_(std::random_device{}()) {}

  // Load all sample data — returns {customers_created, invoices_created}
  std::pair<int, int> load(int target_customers = 100,
                           int target_invoices = 500) {
    std::cout << "Loading sample data...\n";

    int customers_created = 0;
    int invoices_created = 0;

    // Generate customers
    std::vector<int64_t> customer_ids;
    for (int i = 0; i < target_customers; ++i) {
      try {
        auto c = generate_customer(i);
        customer_ids.push_back(c.id);
        customers_created++;
        if (customers_created % 10 == 0)
          std::cout << "  Created " << customers_created << " customers...\n";
      } catch (...) { /* skip duplicates */
      }
    }

    // Generate invoices in batches
    int per_customer = target_invoices / static_cast<int>(customer_ids.size());
    if (per_customer < 1)
      per_customer = 1;

    for (auto cid : customer_ids) {
      int count =
          std::uniform_int_distribution<>(per_customer, per_customer + 3)(rng_);
      for (int j = 0; j < count && invoices_created < target_invoices; ++j) {
        try {
          generate_invoice(cid);
          invoices_created++;
        } catch (...) { /* skip */
        }
      }
    }

    // Ensure we have at least target_invoices
    while (invoices_created < target_invoices && !customer_ids.empty()) {
      int64_t cid = customer_ids[invoices_created % customer_ids.size()];
      try {
        generate_invoice(cid);
        invoices_created++;
      } catch (...) {
        break;
      }
    }

    std::cout << "Sample data loaded: " << customers_created << " customers, "
              << invoices_created << " invoices.\n";
    return {customers_created, invoices_created};
  }

private:
  models::Customer generate_customer(int idx) {
    static const std::vector<std::string> first_names = {
        "James",     "Emma",      "Liam",    "Olivia",  "Noah",
        "Ava",       "William",   "Sophia",  "Mason",   "Isabella",
        "Oliver",    "Charlotte", "Ethan",   "Amelia",  "Aiden",
        "Mia",       "Lucas",     "Harper",  "Logan",   "Evelyn",
        "Alexander", "Abigail",   "Jackson", "Emily",   "Sebastian",
        "Elizabeth", "Jack",      "Mila",    "Owen",    "Ella",
        "Henry",     "Scarlett",  "Carter",  "Aria",    "Wyatt",
        "Luna",      "John",      "Sofia",   "Rajesh",  "Priya",
        "Wei",       "Mei",       "Carlos",  "Maria",   "Mohammed",
        "Fatima",    "David",     "Sarah",   "Michael", "Jennifer"};
    static const std::vector<std::string> last_names = {
        "Smith",     "Johnson", "Williams",  "Brown",     "Jones",
        "Garcia",    "Miller",  "Davis",     "Rodriguez", "Martinez",
        "Hernandez", "Lopez",   "Gonzalez",  "Wilson",    "Anderson",
        "Thomas",    "Taylor",  "Moore",     "Jackson",   "Martin",
        "Lee",       "Perez",   "Thompson",  "White",     "Harris",
        "Sanchez",   "Clark",   "Ramirez",   "Lewis",     "Robinson",
        "Walker",    "Young",   "Allen",     "King",      "Kumar",
        "Patel",     "Shah",    "Sharma",    "Zhang",     "Wang",
        "Chen",      "Liu",     "Fernandez", "Torres",    "Hill",
        "Scott",     "Adams",   "Baker",     "Nelson"};
    static const std::vector<std::string> domains = {
        "gmail.com",      "yahoo.com",    "outlook.com", "company.com",
        "enterprise.org", "business.net", "corp.io",     "tech.ai"};
    static const std::vector<std::pair<std::string, std::string>> locations = {
        {"US", "CA"}, {"US", "NY"}, {"US", "TX"}, {"US", "FL"}, {"US", "WA"},
        {"IN", "MH"}, {"IN", "KA"}, {"IN", "DL"}, {"UK", ""},   {"DE", ""},
        {"FR", ""},   {"SG", ""},   {"AE", ""}};

    auto &fn = first_names[idx % first_names.size()];
    auto &ln = last_names[(idx * 7 + 3) % last_names.size()];
    auto &dom = domains[idx % domains.size()];
    auto &loc = locations[idx % locations.size()];

    service::CustomerCreateRequest req;
    req.name = fn + " " + ln;
    req.email = fn + "." + ln + std::to_string(idx) + "@" + dom;
    req.phone = "+1-555-" + std::to_string(1000 + idx);
    req.address = std::to_string(100 + idx) + " Main St";
    req.country = loc.first;
    req.state = loc.second;

    auto c = cust_svc_.create(req);

    // Randomize some attributes to make data varied
    // Adjust credit score
    std::uniform_int_distribution<> score_dist(400, 820);
    // This is done via direct access (sample data only)
    // In production, use the service method
    return c;
  }

  void generate_invoice(int64_t customer_id) {
    std::uniform_int_distribution<> type_dist(0, 2);
    std::uniform_int_distribution<> item_dist(1, 4);
    std::uniform_real_distribution<> price_dist(50.0, 2000.0);
    std::uniform_int_distribution<> qty_dist(1, 10);
    std::uniform_int_distribution<> due_dist(7, 45);

    static const std::vector<std::string> services = {
        "Software License",   "Support Contract",
        "Consulting Hours",   "Cloud Storage (GB)",
        "API Calls (1K)",     "SLA Extension",
        "Training Session",   "Data Migration",
        "Setup Fee",          "Monthly Subscription",
        "Annual Renewal",     "Custom Development",
        "Security Audit",     "Performance Optimization",
        "Integration Service"};

    service::InvoiceRequest req;
    req.customer_id = customer_id;
    req.currency = "USD";
    req.due_days = std::uniform_int_distribution<>(7, 45)(rng_);

    // Randomly select invoice type
    int type_roll = type_dist(rng_);
    req.type = (type_roll == 0)   ? models::InvoiceType::ONE_TIME
               : (type_roll == 1) ? models::InvoiceType::RECURRING
                                  : models::InvoiceType::PRORATED;

    if (req.type == models::InvoiceType::RECURRING) {
      std::uniform_int_distribution<> period_dist(1, 3);
      int p = period_dist(rng_);
      req.period = (p == 1)   ? models::RecurringPeriod::MONTHLY
                   : (p == 2) ? models::RecurringPeriod::WEEKLY
                              : models::RecurringPeriod::YEARLY;
    }

    if (req.type == models::InvoiceType::PRORATED) {
      std::time_t now = std::time(nullptr);
      req.period_start =
          now - std::uniform_int_distribution<>(5, 25)(rng_) * 86400;
      req.period_end = now;
    }

    // Generate line items
    int num_items = item_dist(rng_);
    for (int i = 0; i < num_items; ++i) {
      models::LineItem li;
      li.description = services[std::uniform_int_distribution<>(
          0, static_cast<int>(services.size()) - 1)(rng_)];
      li.quantity = qty_dist(rng_);
      li.unit_price = price_dist(rng_);
      req.line_items.push_back(li);
    }

    billing_eng_.create_invoice(req);
  }

  service::CustomerService &cust_svc_;
  service::BillingEngine &billing_eng_;
  std::mt19937 rng_;
};

} // namespace billing::data
