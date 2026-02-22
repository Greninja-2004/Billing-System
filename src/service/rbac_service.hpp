#pragma once
// =============================================================================
// rbac_service.hpp — Role-Based Access Control with Bitmask Permissions
// Complexity: Permission check O(1), Role lookup O(1)
// =============================================================================
#include "../core/encryption.hpp"
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace billing::service {

// ---------------------------------------------------------------------------
// Permission bitmasks
// ---------------------------------------------------------------------------
namespace Permission {
constexpr uint32_t READ_CUSTOMER = 1 << 0;     // 0x001
constexpr uint32_t WRITE_CUSTOMER = 1 << 1;    // 0x002
constexpr uint32_t DELETE_CUSTOMER = 1 << 2;   // 0x004
constexpr uint32_t READ_INVOICE = 1 << 3;      // 0x008
constexpr uint32_t WRITE_INVOICE = 1 << 4;     // 0x010
constexpr uint32_t DELETE_INVOICE = 1 << 5;    // 0x020
constexpr uint32_t PROCESS_PAYMENT = 1 << 6;   // 0x040
constexpr uint32_t ISSUE_REFUND = 1 << 7;      // 0x080
constexpr uint32_t VIEW_REPORTS = 1 << 8;      // 0x100
constexpr uint32_t EXPORT_DATA = 1 << 9;       // 0x200
constexpr uint32_t VIEW_AUDIT = 1 << 10;       // 0x400
constexpr uint32_t MANAGE_USERS = 1 << 11;     // 0x800
constexpr uint32_t CONFIGURE_SYSTEM = 1 << 12; // 0x1000

// Role presets
constexpr uint32_t ROLE_READ_ONLY = READ_CUSTOMER | READ_INVOICE | VIEW_REPORTS;
constexpr uint32_t ROLE_BILLING =
    ROLE_READ_ONLY | WRITE_INVOICE | PROCESS_PAYMENT | ISSUE_REFUND;
constexpr uint32_t ROLE_MANAGER =
    ROLE_BILLING | WRITE_CUSTOMER | EXPORT_DATA | VIEW_AUDIT;
constexpr uint32_t ROLE_ADMIN = 0xFFFFFFFF; // all permissions
} // namespace Permission

struct User {
  std::string id;
  std::string name;
  std::string role;
  uint32_t permissions;
  uint32_t password_hash;
  bool active;
};

class RBACService {
public:
  RBACService() { seed_default_users(); }

  // Check permission bitmask — O(1)
  bool has_permission(const std::string &user_id, uint32_t required) const {
    auto it = users_.find(user_id);
    if (it == users_.end() || !it->second.active)
      return false;
    return (it->second.permissions & required) == required;
  }

  // Enforce permission — throws if denied
  void enforce(const std::string &user_id, uint32_t required,
               const std::string &action = "") const {
    if (!has_permission(user_id, required))
      throw std::runtime_error(
          "Access denied" + (action.empty() ? "" : " for action: " + action) +
          " (user: " + user_id + ")");
  }

  // Authenticate user — O(1) lookup + O(1) hash compare
  std::optional<User> login(const std::string &user_id,
                            const std::string &password) const {
    auto it = users_.find(user_id);
    if (it == users_.end() || !it->second.active)
      return std::nullopt;
    uint32_t hash = core::simple_hash(password);
    if (it->second.password_hash != hash)
      return std::nullopt;
    return it->second;
  }

  // Create user
  bool create_user(const std::string &admin_id, const User &user,
                   const std::string &password) {
    enforce(admin_id, Permission::MANAGE_USERS, "create_user");
    if (users_.count(user.id))
      throw std::runtime_error("User already exists");
    auto u = user;
    u.password_hash = core::simple_hash(password);
    u.active = true;
    users_[u.id] = u;
    return true;
  }

  bool deactivate_user(const std::string &admin_id,
                       const std::string &target_id) {
    enforce(admin_id, Permission::MANAGE_USERS);
    auto it = users_.find(target_id);
    if (it == users_.end())
      return false;
    it->second.active = false;
    return true;
  }

  void grant(const std::string &admin_id, const std::string &user_id,
             uint32_t perm) {
    enforce(admin_id, Permission::MANAGE_USERS);
    users_[user_id].permissions |= perm;
  }

  void revoke(const std::string &admin_id, const std::string &user_id,
              uint32_t perm) {
    enforce(admin_id, Permission::MANAGE_USERS);
    users_[user_id].permissions &= ~perm;
  }

  std::optional<User> get_user(const std::string &id) const {
    auto it = users_.find(id);
    if (it == users_.end())
      return std::nullopt;
    return it->second;
  }

  std::vector<User> list_users() const {
    std::vector<User> result;
    for (auto &[id, u] : users_)
      result.push_back(u);
    return result;
  }

  // Describe permissions as string
  static std::string describe_permissions(uint32_t perms) {
    std::string s;
    if (perms & Permission::READ_CUSTOMER)
      s += "read_customer ";
    if (perms & Permission::WRITE_CUSTOMER)
      s += "write_customer ";
    if (perms & Permission::DELETE_CUSTOMER)
      s += "delete_customer ";
    if (perms & Permission::READ_INVOICE)
      s += "read_invoice ";
    if (perms & Permission::WRITE_INVOICE)
      s += "write_invoice ";
    if (perms & Permission::PROCESS_PAYMENT)
      s += "process_payment ";
    if (perms & Permission::ISSUE_REFUND)
      s += "issue_refund ";
    if (perms & Permission::VIEW_REPORTS)
      s += "view_reports ";
    if (perms & Permission::EXPORT_DATA)
      s += "export_data ";
    if (perms & Permission::VIEW_AUDIT)
      s += "view_audit ";
    if (perms & Permission::MANAGE_USERS)
      s += "manage_users ";
    if (perms & Permission::CONFIGURE_SYSTEM)
      s += "configure_system ";
    return s.empty() ? "(none)" : s;
  }

private:
  void seed_default_users() {
    // Admin user: password = "admin123"
    users_["admin"] = {"admin",
                       "System Admin",
                       "ADMIN",
                       Permission::ROLE_ADMIN,
                       core::simple_hash("admin123"),
                       true};
    // Manager: password = "manager123"
    users_["manager"] = {"manager",
                         "Billing Manager",
                         "MANAGER",
                         Permission::ROLE_MANAGER,
                         core::simple_hash("manager123"),
                         true};
    // Agent: password = "agent123"
    users_["agent1"] = {"agent1",
                        "Billing Agent",
                        "BILLING",
                        Permission::ROLE_BILLING,
                        core::simple_hash("agent123"),
                        true};
    // Read-only: password = "readonly"
    users_["viewer"] = {"viewer",
                        "Report Viewer",
                        "READ_ONLY",
                        Permission::ROLE_READ_ONLY,
                        core::simple_hash("readonly"),
                        true};
  }

  std::unordered_map<std::string, User> users_;
};

} // namespace billing::service
