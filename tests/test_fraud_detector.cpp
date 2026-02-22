// test_fraud_detector.cpp
#include "../src/service/fraud_detector.hpp"
#include "test_harness.hpp"

void run_fraud_detector_tests(billing::test::TestSuite &suite) {
  using billing::service::FraudDetector;

  suite.run("FraudDetector: Normal transaction is not flagged", [] {
    FraudDetector fd(60, 10, 5000.0);
    auto sig = fd.check(1, 100.0);
    ASSERT_FALSE(sig.flagged);
    ASSERT_NEAR(sig.risk_score, 0.0, 0.1);
  });

  suite.run("FraudDetector: High frequency triggers flag", [] {
    FraudDetector fd(60, 5, 5000.0); // max 5 tx
    for (int i = 0; i < 6; ++i)
      fd.check(1, 50.0);
    auto sig = fd.check(1, 50.0);
    ASSERT_TRUE(sig.flagged);
  });

  suite.run("FraudDetector: Very large amount raises risk score", [] {
    FraudDetector fd(60, 10, 1000.0);
    auto sig = fd.check(2, 15000.0); // 3x threshold → flagged
    ASSERT_TRUE(sig.flagged);
    ASSERT_GT(sig.risk_score, 0.3);
  });

  suite.run("FraudDetector: Risk score capped at 1.0", [] {
    FraudDetector fd(60, 2, 100.0);
    for (int i = 0; i < 5; ++i)
      fd.check(3, 10000.0);
    auto sig = fd.check(3, 10000.0);
    ASSERT_LT(sig.risk_score, 1.01);
  });

  suite.run("FraudDetector: Transaction count resets per customer", [] {
    FraudDetector fd(60, 10, 5000.0);
    fd.check(10, 100.0);
    fd.check(10, 100.0);
    fd.check(20, 100.0); // different customer
    ASSERT_EQ(fd.transaction_count(10), 2);
    ASSERT_EQ(fd.transaction_count(20), 1);
  });

  suite.run("FraudDetector: Different customers are independent", [] {
    FraudDetector fd(60, 3, 500.0); // max 3 tx
    for (int i = 0; i < 4; ++i)
      fd.check(100, 50.0);             // customer 100 flagged
    auto sig200 = fd.check(200, 50.0); // customer 200 should be fine
    ASSERT_FALSE(sig200.flagged);
  });
}
