// test_snowflake.cpp
#include "../src/core/snowflake.hpp"
#include "test_harness.hpp"
#include <thread>
#include <unordered_set>
#include <vector>

void run_snowflake_tests(billing::test::TestSuite &suite) {
  using billing::core::SnowflakeGenerator;

  suite.run("Snowflake: Generates positive IDs", [] {
    SnowflakeGenerator gen(1);
    auto id = gen.next();
    ASSERT_GT(id, 0LL);
  });

  suite.run("Snowflake: Sequential IDs are monotonically increasing", [] {
    SnowflakeGenerator gen(1);
    auto id1 = gen.next();
    auto id2 = gen.next();
    auto id3 = gen.next();
    ASSERT_GT(id2, id1);
    ASSERT_GT(id3, id2);
  });

  suite.run("Snowflake: 1000 IDs are all unique", [] {
    SnowflakeGenerator gen(1);
    std::unordered_set<int64_t> ids;
    for (int i = 0; i < 1000; ++i)
      ids.insert(gen.next());
    ASSERT_EQ(ids.size(), 1000u);
  });

  suite.run("Snowflake: Concurrent generation produces unique IDs", [] {
    SnowflakeGenerator gen(2);
    std::vector<int64_t> ids(200);
    std::vector<std::thread> threads;
    std::mutex m;
    int idx = 0;
    for (int t = 0; t < 4; ++t) {
      threads.emplace_back([&]() {
        for (int i = 0; i < 50; ++i) {
          auto id = gen.next();
          std::lock_guard<std::mutex> lk(m);
          ids[idx++] = id;
        }
      });
    }
    for (auto &th : threads)
      th.join();
    std::unordered_set<int64_t> unique(ids.begin(), ids.end());
    ASSERT_EQ(unique.size(), 200u);
  });

  suite.run("Snowflake: Decode worker ID correctly", [] {
    SnowflakeGenerator gen(42);
    auto id = gen.next();
    auto decoded = SnowflakeGenerator::decode(id);
    ASSERT_EQ(decoded.worker_id, 42LL);
  });

  suite.run("Snowflake: Invalid worker ID throws", [] {
    ASSERT_THROWS(SnowflakeGenerator bad(1024));
    ASSERT_THROWS(SnowflakeGenerator bad2(-1));
  });
}
