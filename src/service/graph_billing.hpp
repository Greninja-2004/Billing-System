#pragma once
// =============================================================================
// graph_billing.hpp — Graph-Based Billing Chain Dependency Resolution
// Used for: Resolving recurring billing chains, detecting cycles
// Algorithms: BFS (topological processing), Dijkstra (minimum cost path)
// Complexity: BFS O(V+E), Dijkstra O((V+E) log V)
// =============================================================================
#include "../models/invoice.hpp"
#include <functional>
#include <limits>
#include <optional>
#include <queue>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace billing::service {

class BillingGraph {
public:
  // Add a billing dependency: child depends on parent being processed first
  void add_dependency(int64_t parent_id, int64_t child_id,
                      double weight = 1.0) {
    adj_[parent_id].push_back({child_id, weight});
    in_degree_[child_id]++;
    if (in_degree_.find(parent_id) == in_degree_.end())
      in_degree_[parent_id] = 0;
    nodes_.insert(parent_id);
    nodes_.insert(child_id);
  }

  // Add a standalone node (root invoice)
  void add_node(int64_t id) {
    nodes_.insert(id);
    if (in_degree_.find(id) == in_degree_.end())
      in_degree_[id] = 0;
  }

  // BFS Topological sort — returns processing order, O(V+E)
  // Throws if cycle detected
  std::vector<int64_t> topological_sort() const {
    std::unordered_map<int64_t, int> degree = in_degree_;
    std::queue<int64_t> q;
    for (auto &[id, deg] : degree)
      if (deg == 0)
        q.push(id);

    std::vector<int64_t> order;
    while (!q.empty()) {
      int64_t node = q.front();
      q.pop();
      order.push_back(node);
      auto it = adj_.find(node);
      if (it != adj_.end()) {
        for (auto &[child, _] : it->second) {
          if (--degree[child] == 0)
            q.push(child);
        }
      }
    }

    if (order.size() != nodes_.size())
      throw std::runtime_error("Billing dependency cycle detected!");
    return order;
  }

  // BFS reachability — find all invoices reachable from a root, O(V+E)
  std::vector<int64_t> bfs_reachable(int64_t root) const {
    std::unordered_set<int64_t> visited;
    std::queue<int64_t> q;
    q.push(root);
    visited.insert(root);
    std::vector<int64_t> result;

    while (!q.empty()) {
      int64_t curr = q.front();
      q.pop();
      result.push_back(curr);
      auto it = adj_.find(curr);
      if (it != adj_.end()) {
        for (auto &[next, _] : it->second) {
          if (!visited.count(next)) {
            visited.insert(next);
            q.push(next);
          }
        }
      }
    }
    return result;
  }

  // Dijkstra — minimum cost path from src to dst, O((V+E) log V)
  // Used for: finding minimum-cost billing path in weighted chains
  struct DijkstraResult {
    double total_cost;
    std::vector<int64_t> path;
    bool reachable;
  };

  DijkstraResult dijkstra(int64_t src, int64_t dst) const {
    using PII = std::pair<double, int64_t>;
    std::priority_queue<PII, std::vector<PII>, std::greater<>> pq;
    std::unordered_map<int64_t, double> dist;
    std::unordered_map<int64_t, int64_t> prev;

    for (auto &n : nodes_)
      dist[n] = std::numeric_limits<double>::infinity();
    dist[src] = 0.0;
    pq.push({0.0, src});

    while (!pq.empty()) {
      auto [d, u] = pq.top();
      pq.pop();
      if (d > dist[u])
        continue;
      if (u == dst)
        break;

      auto it = adj_.find(u);
      if (it == adj_.end())
        continue;
      for (auto &[v, w] : it->second) {
        double nd = dist[u] + w;
        if (nd < dist[v]) {
          dist[v] = nd;
          prev[v] = u;
          pq.push({nd, v});
        }
      }
    }

    DijkstraResult res;
    res.reachable = dist[dst] < std::numeric_limits<double>::infinity();
    res.total_cost = res.reachable ? dist[dst] : -1;

    // Reconstruct path
    if (res.reachable) {
      int64_t cur = dst;
      while (cur != src) {
        res.path.push_back(cur);
        cur = prev[cur];
      }
      res.path.push_back(src);
      std::reverse(res.path.begin(), res.path.end());
    }
    return res;
  }

  // Detect cycles — O(V+E)
  bool has_cycle() const {
    std::unordered_map<int64_t, int> degree = in_degree_;
    std::queue<int64_t> q;
    for (auto &[id, d] : degree)
      if (d == 0)
        q.push(id);
    int processed = 0;
    while (!q.empty()) {
      int64_t node = q.front();
      q.pop();
      processed++;
      auto it = adj_.find(node);
      if (it != adj_.end())
        for (auto &[child, _] : it->second)
          if (--degree[child] == 0)
            q.push(child);
    }
    return processed != static_cast<int>(nodes_.size());
  }

  void clear() {
    adj_.clear();
    in_degree_.clear();
    nodes_.clear();
  }

  std::size_t node_count() const { return nodes_.size(); }
  std::size_t edge_count() const {
    std::size_t e = 0;
    for (auto &[_, edges] : adj_)
      e += edges.size();
    return e;
  }

private:
  struct Edge {
    int64_t to;
    double weight;
  };
  std::unordered_map<int64_t, std::vector<Edge>> adj_;
  std::unordered_map<int64_t, int> in_degree_;
  std::unordered_set<int64_t> nodes_;
};

} // namespace billing::service
