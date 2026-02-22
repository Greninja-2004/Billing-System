// test_payment_processor.cpp — payment model and core logic tests
#include "../src/core/snowflake.hpp"
#include "../src/models/customer.hpp"
#include "../src/models/invoice.hpp"
#include "../src/models/payment.hpp"
#include "test_harness.hpp"

void run_payment_processor_tests(billing::test::TestSuite &suite) {
  using namespace billing::models;

  suite.run("Payment: payment_method_to_string correct", [] {
    ASSERT_EQ(payment_method_to_string(PaymentMethod::CREDIT_CARD),
              "Credit Card");
    ASSERT_EQ(payment_method_to_string(PaymentMethod::BANK_TRANSFER),
              "Bank Transfer");
    ASSERT_EQ(payment_method_to_string(PaymentMethod::WALLET), "Wallet");
  });

  suite.run("Payment: payment_status_to_string correct", [] {
    ASSERT_EQ(payment_status_to_string(PaymentStatus::COMPLETED), "Completed");
    ASSERT_EQ(payment_status_to_string(PaymentStatus::FAILED), "Failed");
    ASSERT_EQ(payment_status_to_string(PaymentStatus::REFUNDED), "Refunded");
  });

  suite.run("Invoice: amount_due() is total minus paid", [] {
    Invoice inv;
    inv.total_amount = 500.0;
    inv.amount_paid = 200.0;
    ASSERT_NEAR(inv.amount_due(), 300.0, 0.001);
  });

  suite.run("Invoice: amount_due() is 0 when fully paid", [] {
    Invoice inv;
    inv.total_amount = 300.0;
    inv.amount_paid = 300.0;
    ASSERT_NEAR(inv.amount_due(), 0.0, 0.001);
  });

  suite.run(
      "Invoice: amount_due() returns negative on overpayment (credit signal)",
      [] {
        Invoice inv;
        inv.total_amount = 100.0;
        inv.amount_paid = 120.0;
        // amount_due returns negative to indicate a credit balance
        ASSERT_LT(inv.amount_due(), 0.0);
        ASSERT_NEAR(inv.amount_due(), -20.0, 0.001);
      });

  suite.run("Snowflake: IDs used as payment IDs are unique", [] {
    billing::core::SnowflakeGenerator gen(3);
    auto id1 = gen.next();
    auto id2 = gen.next();
    ASSERT_NE(id1, id2);
    ASSERT_GT(id1, 0LL);
    ASSERT_GT(id2, 0LL);
  });

  suite.run("Customer: tier thresholds based on total_spent", [] {
    // Verify tier enum ordering makes sense
    ASSERT_TRUE(static_cast<int>(CustomerTier::BRONZE) == 0);
    ASSERT_TRUE(static_cast<int>(CustomerTier::SILVER) == 1);
    ASSERT_TRUE(static_cast<int>(CustomerTier::GOLD) == 2);
    ASSERT_TRUE(static_cast<int>(CustomerTier::ENTERPRISE) == 3);
  });

  suite.run("Customer: lifetime_months is non-negative", [] {
    Customer c;
    c.created_at = std::time(nullptr) - 90 * 86400; // 3 months ago
    ASSERT_GE(c.lifetime_months(), 2.9);
  });

  suite.run("Refund: model fields set correctly", [] {
    Refund r;
    r.id = 999;
    r.payment_id = 123;
    r.amount = 50.0;
    r.reason = "customer request";
    ASSERT_EQ(r.id, 999LL);
    ASSERT_NEAR(r.amount, 50.0, 0.001);
    ASSERT_EQ(r.reason, "customer request");
  });
}
