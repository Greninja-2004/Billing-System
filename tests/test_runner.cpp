// test_runner.cpp — Main entry point for all unit tests
#include "test_harness.hpp"
#include <iostream>

// Forward declarations of test suites
void run_bplus_tree_tests(billing::test::TestSuite &);
void run_lru_cache_tests(billing::test::TestSuite &);
void run_snowflake_tests(billing::test::TestSuite &);
void run_billing_engine_tests(billing::test::TestSuite &);
void run_payment_processor_tests(billing::test::TestSuite &);
void run_fraud_detector_tests(billing::test::TestSuite &);
void run_report_service_tests(billing::test::TestSuite &);
void run_rbac_tests(billing::test::TestSuite &);

int main() {
  std::cout << "\n========================================\n";
  std::cout << "  Billing System — Unit Test Runner\n";
  std::cout << "========================================\n";

  int total_passed = 0;
  int total_failed = 0;

  auto run_suite = [&](const std::string &name,
                       void (*fn)(billing::test::TestSuite &)) {
    billing::test::TestSuite suite(name);
    fn(suite);
    suite.print_report();
    total_passed += suite.passed();
    total_failed += suite.failed();
  };

  run_suite("B+ Tree", run_bplus_tree_tests);
  run_suite("LRU Cache", run_lru_cache_tests);
  run_suite("Snowflake ID", run_snowflake_tests);
  run_suite("Billing Engine", run_billing_engine_tests);
  run_suite("Payment Processor", run_payment_processor_tests);
  run_suite("Fraud Detector", run_fraud_detector_tests);
  run_suite("Report Service", run_report_service_tests);
  run_suite("RBAC", run_rbac_tests);

  std::cout << "\n========================================\n";
  std::cout << "  TOTAL: " << total_passed << " passed, " << total_failed
            << " failed\n";
  std::cout << "========================================\n";

  return total_failed > 0 ? 1 : 0;
}
