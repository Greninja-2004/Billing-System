#pragma once
// =============================================================================
// admin_cli.hpp — Security, Audit & Administration CLI Module
// =============================================================================
#include "../service/audit_service.hpp"
#include "../service/graph_billing.hpp"
#include "../service/notification_service.hpp"
#include "../service/rbac_service.hpp"
#include "cli_helpers.hpp"
#include <iomanip>
#include <iostream>

namespace billing::cli {

class AdminCLI {
public:
  AdminCLI(service::RBACService &rbac, service::NotificationService &notif,
           const std::string &current_user)
      : rbac_(rbac), notif_(notif), user_(current_user) {}

  void run() {
    while (true) {
      print_header("Security & Administration");
      std::cout << "  [1]  Audit Log Viewer\n"
                << "  [2]  Verify Audit Integrity\n"
                << "  [3]  List Users & Permissions\n"
                << "  [4]  Create User\n"
                << "  [5]  Deactivate User\n"
                << "  [6]  Grant Permission\n"
                << "  [7]  Revoke Permission\n"
                << "  [8]  Dispatch All Notifications\n"
                << "  [9]  Notification Queue Status\n"
                << " [10]  Graph Billing Cycle Detector\n"
                << " [11]  Encryption Demo\n"
                << "  [0]  Back\n";
      print_divider();
      int choice = get_int_input("Select option: ", 0, 11);
      switch (choice) {
      case 0:
        return;
      case 1:
        audit_viewer();
        break;
      case 2:
        verify_audit();
        break;
      case 3:
        list_users();
        break;
      case 4:
        create_user();
        break;
      case 5:
        deactivate_user();
        break;
      case 6:
        grant_perm();
        break;
      case 7:
        revoke_perm();
        break;
      case 8:
        dispatch_notifications();
        break;
      case 9:
        notification_status();
        break;
      case 10:
        graph_demo();
        break;
      case 11:
        encryption_demo();
        break;
      }
    }
  }

private:
  void audit_viewer() {
    try {
      rbac_.enforce(user_, service::Permission::VIEW_AUDIT);
      auto &audit = service::AuditService::instance();
      auto logs = audit.read_all();
      print_header("Audit Log (" + std::to_string(logs.size()) + " entries)");
      int show = std::min(30, static_cast<int>(logs.size()));
      if (show < static_cast<int>(logs.size()))
        print_info("Showing last " + std::to_string(show) + " entries...");
      int start = static_cast<int>(logs.size()) - show;
      for (int i = start; i < static_cast<int>(logs.size()); ++i) {
        auto &e = logs[i];
        std::tm *t = std::localtime(&e.timestamp);
        char buf[32];
        std::strftime(buf, sizeof(buf), "%m-%d %H:%M:%S", t);
        std::cout << std::setw(4) << e.sequence << " | " << buf << " | "
                  << std::setw(10) << e.user_id << " | " << std::setw(12)
                  << models::audit_action_to_string(e.action) << " | "
                  << std::setw(10) << e.entity_type << " | "
                  << e.description.substr(0, 40) << "\n";
      }
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void verify_audit() {
    auto &audit = service::AuditService::instance();
    bool ok = audit.verify_integrity();
    if (ok)
      print_success(
          "Audit log integrity verified: " + std::to_string(audit.count()) +
          " entries, all checksums valid.");
    else
      print_error(
          "INTEGRITY VIOLATION: Some audit entries have been tampered with!");
    press_enter();
  }

  void list_users() {
    try {
      rbac_.enforce(user_, service::Permission::MANAGE_USERS);
      auto users = rbac_.list_users();
      print_header("System Users (" + std::to_string(users.size()) + ")");
      std::cout << std::left << std::setw(12) << "User ID" << std::setw(22)
                << "Name" << std::setw(14) << "Role" << std::setw(8) << "Active"
                << "Permissions\n";
      print_divider();
      for (auto &u : users) {
        std::cout << std::left << std::setw(12) << u.id << std::setw(22)
                  << u.name << std::setw(14) << u.role << std::setw(8)
                  << (u.active ? "YES" : "no") << "0x" << std::hex
                  << u.permissions << std::dec << "\n";
      }
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void create_user() {
    try {
      rbac_.enforce(user_, service::Permission::MANAGE_USERS);
      print_header("Create System User");

      service::User u;
      u.id = get_string_input("User ID: ");
      u.name = get_string_input("Full Name: ");
      u.role = get_string_input("Role (ADMIN/MANAGER/BILLING/READ_ONLY): ");
      u.permissions =
          (u.role == "ADMIN")     ? service::Permission::ROLE_ADMIN
          : (u.role == "MANAGER") ? service::Permission::ROLE_MANAGER
          : (u.role == "BILLING") ? service::Permission::ROLE_BILLING
                                  : service::Permission::ROLE_READ_ONLY;
      u.active = true;
      auto password = get_string_input("Password: ");
      rbac_.create_user(user_, u, password);
      print_success("User created: " + u.id + " (" + u.role + ")");
      AUDIT(user_, models::AuditAction::CREATE, "User", 0,
            "Created user: " + u.id);
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void deactivate_user() {
    try {
      rbac_.enforce(user_, service::Permission::MANAGE_USERS);
      auto target = get_string_input("User ID to deactivate: ");
      if (rbac_.deactivate_user(user_, target)) {
        print_success("User deactivated: " + target);
        AUDIT(user_, models::AuditAction::UPDATE, "User", 0,
              "Deactivated: " + target);
      } else
        print_warning("User not found.");
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void grant_perm() {
    try {
      rbac_.enforce(user_, service::Permission::MANAGE_USERS);
      auto target = get_string_input("User ID: ");
      std::cout << "  Available permissions (hex bitmasks):\n";
      std::cout << "    0x001 READ_CUSTOMER  0x002 WRITE_CUSTOMER  0x040 "
                   "PROCESS_PAYMENT\n";
      std::cout << "    0x100 VIEW_REPORTS   0x200 EXPORT_DATA     0x400 "
                   "VIEW_AUDIT\n";
      std::cout << "    0x800 MANAGE_USERS   0x1000 CONFIG_SYSTEM\n";
      auto hex_str = get_string_input("Permission mask (hex, e.g. 0x100): ");
      uint32_t perm = static_cast<uint32_t>(std::stoul(hex_str, nullptr, 16));
      rbac_.grant(user_, target, perm);
      print_success("Permission 0x" + hex_str + " granted to " + target);
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void revoke_perm() {
    try {
      rbac_.enforce(user_, service::Permission::MANAGE_USERS);
      auto target = get_string_input("User ID: ");
      auto hex_str = get_string_input("Permission mask to revoke (hex): ");
      uint32_t perm = static_cast<uint32_t>(std::stoul(hex_str, nullptr, 16));
      rbac_.revoke(user_, target, perm);
      print_success("Permission revoked from " + target);
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void dispatch_notifications() {
    int count = notif_.dispatch_all();
    print_success("Dispatched " + std::to_string(count) + " notifications.");
    press_enter();
  }

  void notification_status() {
    print_header("Notification Queue Status");
    std::cout << "  Queued:       " << notif_.queue_size() << "\n"
              << "  Sent (total): " << notif_.sent_log().size() << "\n";
    if (!notif_.sent_log().empty()) {
      print_divider();
      std::cout << Color::BOLD << "  Last 5 sent:\n" << Color::RESET;
      auto &log = notif_.sent_log();
      int start = std::max(0, static_cast<int>(log.size()) - 5);
      for (int i = start; i < static_cast<int>(log.size()); ++i) {
        auto &n = log[i];
        std::cout << "    ["
                  << (n.channel == models::NotificationChannel::EMAIL ? "EMAIL"
                      : n.channel == models::NotificationChannel::SMS ? "SMS"
                                                                      : "APP")
                  << "] " << n.subject << "\n";
      }
    }
    press_enter();
  }

  void graph_demo() {
    print_header("Graph Billing Chain Dependency Demo");
    service::BillingGraph g;
    // Build a sample billing chain
    g.add_node(1001);
    g.add_dependency(1001, 1002, 1.0);
    g.add_dependency(1001, 1003, 2.0);
    g.add_dependency(1002, 1004, 1.5);
    g.add_dependency(1003, 1004, 1.0);

    std::cout << "  Nodes: " << g.node_count() << ", Edges: " << g.edge_count()
              << "\n"
              << "  Has cycle: " << (g.has_cycle() ? "YES ⚠" : "No ✓")
              << "\n\n";

    try {
      auto order = g.topological_sort();
      std::cout << Color::BOLD << "  BFS Topological Processing Order:\n"
                << Color::RESET;
      for (auto id : order)
        std::cout << "    Invoice " << id << " → process\n";
    } catch (const std::exception &e) {
      print_error(e.what());
    }

    std::cout << "\n";
    auto path = g.dijkstra(1001, 1004);
    std::cout << Color::BOLD << "  Dijkstra: Minimum cost path 1001→1004:\n"
              << Color::RESET;
    if (path.reachable) {
      std::cout << "  Cost: " << path.total_cost << " | Path: ";
      for (auto id : path.path)
        std::cout << id << " → ";
      std::cout << "END\n";
    }

    auto reachable = g.bfs_reachable(1001);
    std::cout << "\n  BFS Reachable from 1001: ";
    for (auto id : reachable)
      std::cout << id << " ";
    std::cout << "\n";
    press_enter();
  }

  void encryption_demo() {
    print_header("Encryption Demo (XOR Cipher)");
    auto plaintext = get_string_input("Enter text to encrypt: ");
    auto &cipher = core::get_cipher();
    auto encrypted = cipher.encrypt_hex(plaintext);
    auto decrypted = cipher.decrypt_hex(encrypted);
    std::cout << "  Original:  " << plaintext << "\n"
              << "  Encrypted: " << Color::YELLOW << encrypted << Color::RESET
              << "\n"
              << "  Decrypted: " << Color::GREEN << decrypted << Color::RESET
              << "\n";

    // Also show Caesar cipher
    core::CaesarCipher caesar(13); // ROT13
    auto caesar_enc = caesar.encrypt(plaintext);
    std::cout << "  ROT13:     " << Color::CYAN << caesar_enc << Color::RESET
              << "\n";
    press_enter();
  }

  service::RBACService &rbac_;
  service::NotificationService &notif_;
  std::string user_;
};

} // namespace billing::cli
