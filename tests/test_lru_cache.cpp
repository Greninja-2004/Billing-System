// test_lru_cache.cpp
#include "../src/core/lru_cache.hpp"
#include "test_harness.hpp"

void run_lru_cache_tests(billing::test::TestSuite &suite) {
  using billing::core::LRUCache;

  suite.run("LRUCache: Basic put and get", [] {
    LRUCache<int, std::string> cache(5);
    cache.put(1, "one");
    auto v = cache.get(1);
    ASSERT_TRUE(v.has_value());
    ASSERT_EQ(*v, "one");
  });

  suite.run("LRUCache: Get missing key returns nullopt", [] {
    LRUCache<int, int> cache(5);
    ASSERT_FALSE(cache.get(99).has_value());
  });

  suite.run("LRUCache: Evicts LRU when full", [] {
    LRUCache<int, int> cache(3);
    cache.put(1, 1);
    cache.put(2, 2);
    cache.put(3, 3);
    // Access 1 to make it recently used
    cache.get(1);
    // Insert 4, should evict 2 (LRU)
    cache.put(4, 4);
    ASSERT_FALSE(cache.get(2).has_value());
    ASSERT_TRUE(cache.get(1).has_value());
    ASSERT_TRUE(cache.get(3).has_value());
    ASSERT_TRUE(cache.get(4).has_value());
  });

  suite.run("LRUCache: Update existing key moves to front", [] {
    LRUCache<int, int> cache(3);
    cache.put(1, 1);
    cache.put(2, 2);
    cache.put(3, 3);
    cache.put(1, 100); // update — should move to MRU
    cache.put(4, 4);   // evicts LRU = 2
    ASSERT_FALSE(cache.get(2).has_value());
    ASSERT_EQ(*cache.get(1), 100);
  });

  suite.run("LRUCache: Hit rate tracking", [] {
    LRUCache<int, int> cache(5);
    cache.put(1, 1);
    cache.put(2, 2);
    cache.get(1);  // hit
    cache.get(99); // miss
    ASSERT_NEAR(cache.hit_rate(), 0.5, 0.001);
  });

  suite.run("LRUCache: Evict specific key", [] {
    LRUCache<int, int> cache(5);
    cache.put(1, 1);
    bool evicted = cache.evict(1);
    ASSERT_TRUE(evicted);
    ASSERT_FALSE(cache.get(1).has_value());
    ASSERT_FALSE(cache.evict(99));
  });

  suite.run("LRUCache: Contains check", [] {
    LRUCache<int, int> cache(5);
    cache.put(42, 100);
    ASSERT_TRUE(cache.contains(42));
    ASSERT_FALSE(cache.contains(1));
  });

  suite.run("LRUCache: Throws on zero capacity", [] {
    using IntCache = LRUCache<int, int>;
    ASSERT_THROWS(IntCache c(0));
  });
}
