#pragma once
// =============================================================================
// test_harness.hpp — Lightweight Unit Test Framework
// =============================================================================
#include <cmath>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace billing::test {

struct TestResult {
  std::string name;
  bool passed;
  std::string message;
};

class TestSuite {
public:
  explicit TestSuite(const std::string &suite_name) : name_(suite_name) {}

  void run(const std::string &test_name, std::function<void()> test_fn) {
    total_++;
    try {
      test_fn();
      results_.push_back({test_name, true, "OK"});
      passed_++;
    } catch (const std::exception &e) {
      results_.push_back({test_name, false, e.what()});
    } catch (...) {
      results_.push_back({test_name, false, "Unknown exception"});
    }
  }

  void print_report() const {
    std::cout << "\n=== " << name_ << " ===" << std::endl;
    for (auto &r : results_) {
      std::cout << (r.passed ? "  [PASS] " : "  [FAIL] ") << r.name;
      if (!r.passed)
        std::cout << "\n         → " << r.message;
      std::cout << "\n";
    }
    std::cout << "  " << passed_ << "/" << total_ << " passed\n";
  }

  int failed() const { return total_ - passed_; }
  int total() const { return total_; }
  int passed() const { return passed_; }

private:
  std::string name_;
  std::vector<TestResult> results_;
  int total_ = 0;
  int passed_ = 0;
};

// Assertion macros
#define ASSERT_TRUE(expr)                                                      \
  if (!(expr))                                                                 \
  throw std::runtime_error(std::string("ASSERT_TRUE failed: ") + #expr +       \
                           " at line " + std::to_string(__LINE__))

#define ASSERT_FALSE(expr)                                                     \
  if ((expr))                                                                  \
  throw std::runtime_error(std::string("ASSERT_FALSE failed: ") + #expr)

#define ASSERT_EQ(a, b)                                                        \
  if (!((a) == (b))) {                                                         \
    std::ostringstream oss;                                                    \
    oss << "ASSERT_EQ failed: " << (a) << " != " << (b)                        \
        << " (" #a " != " #b ")";                                              \
    throw std::runtime_error(oss.str());                                       \
  }

#define ASSERT_NE(a, b)                                                        \
  if ((a) == (b))                                                              \
  throw std::runtime_error(std::string("ASSERT_NE failed: " #a " == " #b))

#define ASSERT_GT(a, b)                                                        \
  if (!((a) > (b))) {                                                          \
    std::ostringstream oss;                                                    \
    oss << "ASSERT_GT failed: " << (a) << " <= " << (b);                       \
    throw std::runtime_error(oss.str());                                       \
  }

#define ASSERT_GE(a, b)                                                        \
  if (!((a) >= (b))) {                                                         \
    std::ostringstream oss;                                                    \
    oss << "ASSERT_GE failed: " << (a) << " < " << (b);                        \
    throw std::runtime_error(oss.str());                                       \
  }

#define ASSERT_LT(a, b)                                                        \
  if (!((a) < (b))) {                                                          \
    std::ostringstream oss;                                                    \
    oss << "ASSERT_LT failed: " << (a) << " >= " << (b);                       \
    throw std::runtime_error(oss.str());                                       \
  }

#define ASSERT_NEAR(a, b, eps)                                                 \
  if (std::abs((double)(a) - (double)(b)) > (eps)) {                           \
    std::ostringstream oss;                                                    \
    oss << "ASSERT_NEAR failed: |" << (a) << " - " << (b) << "| > " << (eps);  \
    throw std::runtime_error(oss.str());                                       \
  }

#define ASSERT_THROWS(expr)                                                    \
  {                                                                            \
    bool threw = false;                                                        \
    try {                                                                      \
      expr;                                                                    \
    } catch (...) {                                                            \
      threw = true;                                                            \
    }                                                                          \
    if (!threw)                                                                \
      throw std::runtime_error(                                                \
          "ASSERT_THROWS: no exception thrown for: " #expr);                   \
  }

#define ASSERT_NO_THROW(expr)                                                  \
  try {                                                                        \
    expr;                                                                      \
  } catch (const std::exception &e) {                                          \
    throw std::runtime_error(std::string("ASSERT_NO_THROW: threw: ") +         \
                             e.what());                                        \
  }

} // namespace billing::test
