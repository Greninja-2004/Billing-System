// test_rbac.cpp
#include "../src/service/rbac_service.hpp"
#include "test_harness.hpp"

void run_rbac_tests(billing::test::TestSuite &suite) {
  using namespace billing::service;

  suite.run("RBAC: Admin has all permissions", [] {
    RBACService rbac;
    ASSERT_TRUE(rbac.has_permission("admin", Permission::ROLE_ADMIN));
    ASSERT_TRUE(rbac.has_permission("admin", Permission::MANAGE_USERS));
    ASSERT_TRUE(rbac.has_permission("admin", Permission::CONFIGURE_SYSTEM));
  });

  suite.run("RBAC: Viewer has read-only permissions", [] {
    RBACService rbac;
    ASSERT_TRUE(rbac.has_permission("viewer", Permission::READ_CUSTOMER));
    ASSERT_TRUE(rbac.has_permission("viewer", Permission::VIEW_REPORTS));
    ASSERT_FALSE(rbac.has_permission("viewer", Permission::WRITE_CUSTOMER));
    ASSERT_FALSE(rbac.has_permission("viewer", Permission::PROCESS_PAYMENT));
  });

  suite.run("RBAC: Enforce throws on permission denied", [] {
    RBACService rbac;
    ASSERT_THROWS(rbac.enforce("viewer", Permission::MANAGE_USERS, "test"));
  });

  suite.run("RBAC: Enforce passes when permission granted", [] {
    RBACService rbac;
    ASSERT_NO_THROW(rbac.enforce("admin", Permission::MANAGE_USERS, "test"));
  });

  suite.run("RBAC: Login with correct credentials", [] {
    RBACService rbac;
    auto user = rbac.login("admin", "admin123");
    ASSERT_TRUE(user.has_value());
    ASSERT_EQ(user->id, "admin");
  });

  suite.run("RBAC: Login with wrong password returns nullopt", [] {
    RBACService rbac;
    auto user = rbac.login("admin", "wrongpassword");
    ASSERT_FALSE(user.has_value());
  });

  suite.run("RBAC: Grant and revoke permission", [] {
    RBACService rbac;
    ASSERT_FALSE(rbac.has_permission("viewer", Permission::EXPORT_DATA));
    rbac.grant("admin", "viewer", Permission::EXPORT_DATA);
    ASSERT_TRUE(rbac.has_permission("viewer", Permission::EXPORT_DATA));
    rbac.revoke("admin", "viewer", Permission::EXPORT_DATA);
    ASSERT_FALSE(rbac.has_permission("viewer", Permission::EXPORT_DATA));
  });

  suite.run("RBAC: Bitmask: multiple permissions combined", [] {
    uint32_t combined = Permission::READ_CUSTOMER | Permission::READ_INVOICE;
    RBACService rbac;
    ASSERT_TRUE(rbac.has_permission("viewer", combined));
  });

  suite.run("RBAC: Deactivated user cannot login", [] {
    RBACService rbac;
    rbac.deactivate_user("admin", "agent1");
    ASSERT_FALSE(rbac.has_permission("agent1", Permission::READ_CUSTOMER));
  });
}
