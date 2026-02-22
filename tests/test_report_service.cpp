// test_report_service.cpp — report logic tests
#include "../src/core/min_heap.hpp"
#include "../src/models/customer.hpp"
#include "../src/models/invoice.hpp"
#include "../src/service/graph_billing.hpp"
#include "test_harness.hpp"

void run_report_service_tests(billing::test::TestSuite &suite) {
  using namespace billing;

  suite.run("AgingBucket: days_overdue buckets correctly", [] {
    models::Invoice inv;
    inv.status = models::InvoiceStatus::PENDING;
    inv.total_amount = 100.0;
    inv.amount_paid = 0.0;

    // 10 days overdue → current (0-30)
    inv.due_date = std::time(nullptr) - 10 * 86400;
    ASSERT_EQ(inv.days_overdue(), 10);

    // 45 days overdue → 31-60 bucket
    inv.due_date = std::time(nullptr) - 45 * 86400;
    ASSERT_GE(inv.days_overdue(), 44);

    // Not yet due → 0 days
    inv.due_date = std::time(nullptr) + 5 * 86400;
    ASSERT_EQ(inv.days_overdue(), 0);
  });

  suite.run("MinHeap: invoice sorted by due_date", [] {
    struct Item {
      int64_t due_date;
      int id;
    };
    auto cmp = [](const Item &a, const Item &b) {
      return a.due_date < b.due_date;
    };
    core::MinHeap<Item, decltype(cmp)> heap(cmp);
    heap.push({300LL, 3});
    heap.push({100LL, 1});
    heap.push({200LL, 2});
    auto first = heap.pop();
    ASSERT_EQ(first.id, 1);
    auto second = heap.pop();
    ASSERT_EQ(second.id, 2);
  });

  suite.run("BillingGraph: topological sort on simple chain", [] {
    service::BillingGraph g;
    g.add_dependency(1, 2);
    g.add_dependency(2, 3);
    auto order = g.topological_sort();
    ASSERT_EQ(order.size(), 3u);
    ASSERT_EQ(order[0], 1LL);
    ASSERT_EQ(order[2], 3LL);
  });

  suite.run("BillingGraph: cycle detection", [] {
    service::BillingGraph g;
    g.add_dependency(1, 2);
    g.add_dependency(2, 3);
    g.add_dependency(3, 1); // cycle!
    ASSERT_TRUE(g.has_cycle());
  });

  suite.run("BillingGraph: no cycle on valid chain", [] {
    service::BillingGraph g;
    g.add_dependency(1, 2);
    g.add_dependency(1, 3);
    g.add_dependency(2, 4);
    ASSERT_FALSE(g.has_cycle());
  });

  suite.run("BillingGraph: dijkstra finds shortest path", [] {
    service::BillingGraph g;
    g.add_dependency(1, 2, 1.0);
    g.add_dependency(1, 3, 5.0);
    g.add_dependency(2, 4, 1.0);
    g.add_dependency(3, 4, 1.0);
    auto result = g.dijkstra(1, 4);
    ASSERT_TRUE(result.reachable);
    ASSERT_NEAR(result.total_cost, 2.0, 0.001); // 1→2→4 = 2
  });

  suite.run("BillingGraph: BFS reachability", [] {
    service::BillingGraph g;
    g.add_dependency(1, 2);
    g.add_dependency(1, 3);
    g.add_dependency(2, 4);
    auto reachable = g.bfs_reachable(1);
    ASSERT_EQ(reachable.size(), 4u);
  });
}
