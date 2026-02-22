// test_billing_engine.cpp — Billing engine tests using in-memory mock
#include "../src/core/snowflake.hpp"
#include "../src/models/invoice.hpp"
#include "../src/service/discount_engine.hpp"
#include "../src/service/tax_engine.hpp"
#include "test_harness.hpp"

void run_billing_engine_tests(billing::test::TestSuite &suite) {
  using namespace billing;

  suite.run("DiscountEngine: Percentage discount applies correctly", [] {
    service::DiscountEngine engine;
    models::Customer c;
    c.tier = models::CustomerTier::BRONZE;
    c.created_at = std::time(nullptr);
    c.total_spent = 0;
    models::Invoice inv;
    inv.type = models::InvoiceType::ONE_TIME;
    inv.subtotal = 1000.0;
    double disc = engine.apply(1000.0, c, inv);
    ASSERT_GE(disc, 0.0);
    ASSERT_LT(disc, 500.0); // not more than 50%
  });

  suite.run("DiscountEngine: Enterprise gets higher discount", [] {
    service::DiscountEngine engine;
    models::Customer bronze, enterprise;
    bronze.tier = models::CustomerTier::BRONZE;
    bronze.created_at = std::time(nullptr);
    bronze.total_spent = 0;
    enterprise.tier = models::CustomerTier::ENTERPRISE;
    enterprise.created_at = std::time(nullptr) - 400 * 86400; // 13+ months old
    enterprise.total_spent = 60000;
    models::Invoice inv;
    inv.type = models::InvoiceType::ONE_TIME;
    inv.subtotal = 2000.0;
    double d_bronze = engine.apply(2000.0, bronze, inv);
    double d_enterprise = engine.apply(2000.0, enterprise, inv);
    ASSERT_GT(d_enterprise, d_bronze);
  });

  suite.run("TaxEngine: US-CA tax computes correctly", [] {
    service::TaxEngine tax;
    auto result = tax.compute(1000.0, "US-CA");
    ASSERT_NEAR(result.state_tax, 72.5, 0.1);
    ASSERT_GT(result.total_tax, 0.0);
  });

  suite.run("TaxEngine: IN GST 18% applies", [] {
    service::TaxEngine tax;
    auto result = tax.compute(1000.0, "IN");
    ASSERT_NEAR(result.gst_tax, 180.0, 0.1);
  });

  suite.run("TaxEngine: HK zero tax", [] {
    service::TaxEngine tax;
    auto result = tax.compute(5000.0, "HK");
    ASSERT_NEAR(result.total_tax, 0.0, 0.01);
  });

  suite.run("TaxEngine: Unknown jurisdiction returns zero tax", [] {
    service::TaxEngine tax;
    auto result = tax.compute(1000.0, "NEVER_LAND");
    ASSERT_NEAR(result.total_tax, 0.0, 0.01);
  });

  suite.run("Invoice: is_overdue and days_overdue correct", [] {
    models::Invoice inv;
    inv.status = models::InvoiceStatus::PENDING;
    inv.due_date = std::time(nullptr) - 5 * 86400; // 5 days ago
    inv.amount_paid = 0;
    inv.total_amount = 100;
    ASSERT_TRUE(inv.is_overdue());
    ASSERT_GE(inv.days_overdue(), 4);
  });

  suite.run("Invoice: Paid invoice is not overdue", [] {
    models::Invoice inv;
    inv.status = models::InvoiceStatus::PAID;
    inv.due_date = std::time(nullptr) - 2 * 86400;
    ASSERT_FALSE(inv.is_overdue());
  });
}
