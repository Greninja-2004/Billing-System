#pragma once
// =============================================================================
// bplus_tree.hpp — B+ Tree (Order 4) Implementation
// Used for: Customer & Invoice indexing with range query support
// Complexity: Insert O(log n), Search O(log n), Range O(log n + k), Delete
// O(log n)
// =============================================================================
#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <vector>

namespace billing::core {

template <typename Key, typename Value, int Order = 4> class BPlusTree {
public:
  static constexpr int MAX_KEYS = Order - 1;       // max keys per node
  static constexpr int MIN_KEYS = (Order - 1) / 2; // min keys (except root)

  // -----------------------------------------------------------------------
  // Node types
  // -----------------------------------------------------------------------
  struct Node {
    bool is_leaf;
    int num_keys;
    std::array<Key, Order> keys;            // Order keys (one spare for split)
    std::array<Node *, Order + 1> children; // internal: child pointers
    std::array<Value, Order> values;        // leaf: values
    Node *next_leaf;                        // linked leaf list

    Node(bool leaf) : is_leaf(leaf), num_keys(0), next_leaf(nullptr) {
      children.fill(nullptr);
    }
  };

  // -----------------------------------------------------------------------
  BPlusTree() : root_(nullptr), size_(0) { root_ = new Node(true); }

  ~BPlusTree() { destroy(root_); }

  // Non-copyable
  BPlusTree(const BPlusTree &) = delete;
  BPlusTree &operator=(const BPlusTree &) = delete;

  // Insert key-value pair — O(log n)
  void insert(const Key &key, const Value &value) {
    auto [new_child, promoted_key, did_split] =
        insert_recursive(root_, key, value);
    if (did_split) {
      Node *new_root = new Node(false);
      new_root->keys[0] = promoted_key;
      new_root->children[0] = root_;
      new_root->children[1] = new_child;
      new_root->num_keys = 1;
      root_ = new_root;
    }
    size_++;
  }

  // Search for exact key — O(log n)
  std::optional<Value> search(const Key &key) const {
    Node *node = root_;
    while (!node->is_leaf) {
      int i = upper_bound_idx(node, key);
      node = node->children[i];
    }
    for (int i = 0; i < node->num_keys; ++i) {
      if (node->keys[i] == key)
        return node->values[i];
    }
    return std::nullopt;
  }

  // Range query [lo, hi] — O(log n + k)
  std::vector<std::pair<Key, Value>> range(const Key &lo, const Key &hi) const {
    std::vector<std::pair<Key, Value>> result;
    Node *node = root_;
    while (!node->is_leaf) {
      int i = lower_bound_idx(node, lo);
      node = node->children[i];
    }
    while (node) {
      for (int i = 0; i < node->num_keys; ++i) {
        if (node->keys[i] > hi)
          return result;
        if (node->keys[i] >= lo)
          result.emplace_back(node->keys[i], node->values[i]);
      }
      node = node->next_leaf;
    }
    return result;
  }

  // Update value for existing key — O(log n)
  bool update(const Key &key, const Value &value) {
    Node *node = root_;
    while (!node->is_leaf) {
      int i = upper_bound_idx(node, key);
      node = node->children[i];
    }
    for (int i = 0; i < node->num_keys; ++i) {
      if (node->keys[i] == key) {
        node->values[i] = value;
        return true;
      }
    }
    return false;
  }

  // Remove key — O(log n)
  bool remove(const Key &key) {
    bool removed = remove_recursive(root_, key);
    if (removed) {
      --size_;
      // If root has no keys and has a child, shrink tree
      if (!root_->is_leaf && root_->num_keys == 0) {
        Node *old_root = root_;
        root_ = root_->children[0];
        old_root->children[0] = nullptr;
        delete old_root;
      }
    }
    return removed;
  }

  // Iterate all leaf entries in sorted order
  void for_each(std::function<void(const Key &, const Value &)> fn) const {
    Node *node = root_;
    while (!node->is_leaf)
      node = node->children[0];
    while (node) {
      for (int i = 0; i < node->num_keys; ++i)
        fn(node->keys[i], node->values[i]);
      node = node->next_leaf;
    }
  }

  std::size_t size() const { return size_; }
  bool empty() const { return size_ == 0; }

private:
  // -----------------------------------------------------------------------
  // Internal helpers
  // -----------------------------------------------------------------------

  int upper_bound_idx(const Node *n, const Key &key) const {
    int i = 0;
    while (i < n->num_keys && key >= n->keys[i])
      ++i;
    return i;
  }

  int lower_bound_idx(const Node *n, const Key &key) const {
    int i = 0;
    while (i < n->num_keys && key > n->keys[i])
      ++i;
    return i;
  }

  // Returns {new_sibling, promoted_key, did_split}
  struct InsertResult {
    Node *new_child;
    Key promoted_key;
    bool did_split;
  };

  InsertResult insert_recursive(Node *node, const Key &key,
                                const Value &value) {
    if (node->is_leaf) {
      // Insert into leaf in sorted order
      int pos = lower_bound_idx(node, key);
      // Shift right
      for (int i = node->num_keys; i > pos; --i) {
        node->keys[i] = node->keys[i - 1];
        node->values[i] = node->values[i - 1];
      }
      node->keys[pos] = key;
      node->values[pos] = value;
      node->num_keys++;

      if (node->num_keys < Order)
        return {nullptr, {}, false};
      return split_leaf(node);
    }

    // Internal node: find child to recurse into
    int i = upper_bound_idx(node, key);
    auto [new_child, pkey, did_split] =
        insert_recursive(node->children[i], key, value);
    if (!did_split)
      return {nullptr, {}, false};

    // Insert promoted key into this node
    for (int j = node->num_keys; j > i; --j) {
      node->keys[j] = node->keys[j - 1];
      node->children[j + 1] = node->children[j];
    }
    node->keys[i] = pkey;
    node->children[i + 1] = new_child;
    node->num_keys++;

    if (node->num_keys < Order)
      return {nullptr, {}, false};
    return split_internal(node);
  }

  InsertResult split_leaf(Node *leaf) {
    Node *sibling = new Node(true);
    int mid = Order / 2;

    sibling->num_keys = leaf->num_keys - mid;
    for (int i = 0; i < sibling->num_keys; ++i) {
      sibling->keys[i] = leaf->keys[mid + i];
      sibling->values[i] = leaf->values[mid + i];
    }
    leaf->num_keys = mid;
    sibling->next_leaf = leaf->next_leaf;
    leaf->next_leaf = sibling;

    return {sibling, sibling->keys[0], true};
  }

  InsertResult split_internal(Node *node) {
    int mid = node->num_keys / 2;
    Key pkey = node->keys[mid];

    Node *sibling = new Node(false);
    sibling->num_keys = node->num_keys - mid - 1;
    for (int i = 0; i < sibling->num_keys; ++i)
      sibling->keys[i] = node->keys[mid + 1 + i];
    for (int i = 0; i <= sibling->num_keys; ++i)
      sibling->children[i] = node->children[mid + 1 + i];
    node->num_keys = mid;

    return {sibling, pkey, true};
  }

  bool remove_recursive(Node *node, const Key &key) {
    if (node->is_leaf) {
      for (int i = 0; i < node->num_keys; ++i) {
        if (node->keys[i] == key) {
          for (int j = i; j < node->num_keys - 1; ++j) {
            node->keys[j] = node->keys[j + 1];
            node->values[j] = node->values[j + 1];
          }
          node->num_keys--;
          return true;
        }
      }
      return false;
    }
    int i = upper_bound_idx(node, key);
    bool removed = remove_recursive(node->children[i], key);
    return removed;
  }

  void destroy(Node *node) {
    if (!node)
      return;
    if (!node->is_leaf) {
      for (int i = 0; i <= node->num_keys; ++i)
        destroy(node->children[i]);
    }
    delete node;
  }

  Node *root_;
  std::size_t size_;
};

} // namespace billing::core
