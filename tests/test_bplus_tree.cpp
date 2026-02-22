// test_bplus_tree.cpp
#include "../src/core/bplus_tree.hpp"
#include "test_harness.hpp"

void run_bplus_tree_tests(billing::test::TestSuite &suite) {
  using namespace billing::core;
  using billing::test::TestSuite;

  suite.run("BPlusTree: Insert and search single element", [] {
    BPlusTree<int, std::string> tree;
    tree.insert(1, "hello");
    auto result = tree.search(1);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(*result, "hello");
  });

  suite.run("BPlusTree: Search missing key returns nullopt", [] {
    BPlusTree<int, std::string> tree;
    tree.insert(42, "world");
    auto result = tree.search(99);
    ASSERT_FALSE(result.has_value());
  });

  suite.run("BPlusTree: Insert 50 elements and find all", [] {
    BPlusTree<int, int> tree;
    for (int i = 0; i < 50; ++i)
      tree.insert(i, i * 2);
    ASSERT_EQ(tree.size(), 50u);
    for (int i = 0; i < 50; ++i) {
      auto r = tree.search(i);
      ASSERT_TRUE(r.has_value());
      ASSERT_EQ(*r, i * 2);
    }
  });

  suite.run("BPlusTree: Range query", [] {
    BPlusTree<int, int> tree;
    for (int i = 1; i <= 20; ++i)
      tree.insert(i, i);
    auto result = tree.range(5, 10);
    ASSERT_EQ(result.size(), 6u);
    ASSERT_EQ(result[0].first, 5);
    ASSERT_EQ(result[5].first, 10);
  });

  suite.run("BPlusTree: Update existing key", [] {
    BPlusTree<int, std::string> tree;
    tree.insert(7, "old");
    bool updated = tree.update(7, "new");
    ASSERT_TRUE(updated);
    ASSERT_EQ(*tree.search(7), "new");
  });

  suite.run("BPlusTree: Remove key", [] {
    BPlusTree<int, int> tree;
    for (int i = 1; i <= 10; ++i)
      tree.insert(i, i);
    bool removed = tree.remove(5);
    ASSERT_TRUE(removed);
    ASSERT_EQ(tree.size(), 9u);
    ASSERT_FALSE(tree.search(5).has_value());
  });

  suite.run("BPlusTree: For-each iterates in order", [] {
    BPlusTree<int, int> tree;
    for (int i : {5, 3, 8, 1, 7})
      tree.insert(i, i);
    std::vector<int> keys;
    tree.for_each([&](const int &k, const int &) { keys.push_back(k); });
    ASSERT_EQ(keys.size(), 5u);
    for (std::size_t i = 1; i < keys.size(); ++i)
      ASSERT_LT(keys[i - 1], keys[i]);
  });

  suite.run("BPlusTree: Handle 200 insertions correctly", [] {
    BPlusTree<int64_t, int64_t> tree;
    for (int i = 0; i < 200; ++i)
      tree.insert(i * 7LL, i);
    ASSERT_EQ(tree.size(), 200u);
    for (int i = 0; i < 200; ++i) {
      auto r = tree.search(i * 7LL);
      ASSERT_TRUE(r.has_value());
      ASSERT_EQ(*r, i);
    }
  });
}
