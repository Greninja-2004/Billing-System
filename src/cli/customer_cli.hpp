#pragma once
// =============================================================================
// customer_cli.hpp — Customer Management CLI Module
// =============================================================================
#include "../service/audit_service.hpp"
#include "../service/customer_service.hpp"
#include "../service/rbac_service.hpp"
#include "cli_helpers.hpp"
#include <iomanip>
#include <iostream>

namespace billing::cli {

class CustomerCLI {
public:
  CustomerCLI(service::CustomerService &svc, service::RBACService &rbac,
              const std::string &current_user)
      : svc_(svc), rbac_(rbac), user_(current_user) {}

  void run() {
    while (true) {
      print_header("Customer Management");
      std::cout << "  [1] List All Customers\n"
                << "  [2] Search Customer by ID\n"
                << "  [3] Search Customer by Email\n"
                << "  [4] Create New Customer\n"
                << "  [5] Update Customer Profile\n"
                << "  [6] Suspend / Activate Customer\n"
                << "  [7] Delete Customer\n"
                << "  [8] List by Tier\n"
                << "  [0] Back to Main Menu\n";
      print_divider();

      int choice = get_int_input("Select option: ", 0, 8);
      switch (choice) {
      case 0:
        return;
      case 1:
        list_all();
        break;
      case 2:
        search_by_id();
        break;
      case 3:
        search_by_email();
        break;
      case 4:
        create_customer();
        break;
      case 5:
        update_profile();
        break;
      case 6:
        toggle_status();
        break;
      case 7:
        delete_customer();
        break;
      case 8:
        list_by_tier();
        break;
      }
    }
  }

private:
  void print_customer(const models::Customer &c) {
    print_divider();
    std::cout << Color::BOLD << "Customer ID: " << Color::RESET << c.id << "\n"
              << "  Name:         " << c.name << "\n"
              << "  Email:        " << c.email << "\n"
              << "  Phone:        " << c.phone << "\n"
              << "  Address:      " << c.address << "\n"
              << "  Country/State:" << c.country << "/" << c.state << "\n"
              << "  Tier:         " << Color::YELLOW
              << models::tier_to_string(c.tier) << Color::RESET << "\n"
              << "  Status:       "
              << (c.status == models::CustomerStatus::ACTIVE
                      ? std::string(Color::GREEN) + "Active" + Color::RESET
                      : std::string(Color::RED) + "Suspended" + Color::RESET)
              << "\n"
              << "  Credit Score: " << c.credit_score << "\n"
              << "  Credit Limit: " << format_currency(c.credit_limit) << "\n"
              << "  Balance:      " << format_currency(c.current_balance)
              << "\n"
              << "  Total Spent:  " << format_currency(c.total_spent) << "\n"
              << "  Member Since: " << format_time(c.created_at) << "\n";
  }

  void list_all() {
    try {
      rbac_.enforce(user_, service::Permission::READ_CUSTOMER,
                    "list_customers");
      auto customers = svc_.list_all();
      print_header("All Customers (" + std::to_string(customers.size()) + ")");
      std::cout << std::left << std::setw(20) << "ID" << std::setw(25) << "Name"
                << std::setw(30) << "Email" << std::setw(12) << "Tier"
                << std::setw(10) << "Status" << std::setw(10) << "Score"
                << "\n";
      print_divider();
      for (auto &c : customers) {
        std::cout << std::left << std::setw(20) << c.id << std::setw(25)
                  << c.name.substr(0, 23) << std::setw(30)
                  << c.email.substr(0, 28) << std::setw(12)
                  << models::tier_to_string(c.tier) << std::setw(10)
                  << models::status_to_string(c.status) << std::setw(10)
                  << c.credit_score << "\n";
      }
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void search_by_id() {
    try {
      rbac_.enforce(user_, service::Permission::READ_CUSTOMER, "find_customer");
      int64_t id = get_id_input("Enter Customer ID: ");
      auto opt = svc_.get(id);
      if (opt)
        print_customer(*opt);
      else
        print_warning("No customer found with ID: " + std::to_string(id));
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void search_by_email() {
    try {
      rbac_.enforce(user_, service::Permission::READ_CUSTOMER, "find_customer");
      auto email = get_string_input("Enter Email: ");
      auto opt = svc_.get_by_email(email);
      if (opt)
        print_customer(*opt);
      else
        print_warning("No customer found with email: " + email);
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void create_customer() {
    try {
      rbac_.enforce(user_, service::Permission::WRITE_CUSTOMER,
                    "create_customer");
      print_header("Create New Customer");
      service::CustomerCreateRequest req;
      req.name = get_string_input("Name: ");
      req.email = get_string_input("Email: ");
      req.phone = get_string_input("Phone: ");
      req.address = get_string_input("Address: ");
      req.country = get_string_input("Country Code (US/IN/UK/...): ");
      req.state = get_string_input("State Code (CA/NY/... or blank): ");

      auto c = svc_.create(req);
      print_success("Customer created: " + c.name +
                    " (ID: " + std::to_string(c.id) + ")");
      AUDIT(user_, models::AuditAction::CREATE, "Customer", c.id,
            "Created customer: " + c.name);
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void update_profile() {
    try {
      rbac_.enforce(user_, service::Permission::WRITE_CUSTOMER,
                    "update_customer");
      int64_t id = get_id_input("Customer ID to update: ");
      auto opt = svc_.get(id);
      if (!opt) {
        print_warning("Customer not found");
        press_enter();
        return;
      }
      print_customer(*opt);

      auto name = get_string_input("New Name (blank=keep): ");
      auto phone = get_string_input("New Phone (blank=keep): ");
      auto address = get_string_input("New Address (blank=keep): ");

      if (name.empty())
        name = opt->name;
      if (phone.empty())
        phone = opt->phone;
      if (address.empty())
        address = opt->address;

      if (svc_.update_profile(id, name, phone, address)) {
        print_success("Profile updated.");
        AUDIT(user_, models::AuditAction::UPDATE, "Customer", id,
              "Updated profile");
      } else
        print_error("Update failed.");
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void toggle_status() {
    try {
      int64_t id = get_id_input("Customer ID: ");
      auto opt = svc_.get(id);
      if (!opt) {
        print_warning("Customer not found");
        press_enter();
        return;
      }
      if (opt->status == models::CustomerStatus::ACTIVE) {
        rbac_.enforce(user_, service::Permission::WRITE_CUSTOMER);
        svc_.suspend(id);
        print_success("Customer suspended.");
        AUDIT(user_, models::AuditAction::UPDATE, "Customer", id, "Suspended");
      } else {
        rbac_.enforce(user_, service::Permission::WRITE_CUSTOMER);
        svc_.activate(id);
        print_success("Customer activated.");
        AUDIT(user_, models::AuditAction::UPDATE, "Customer", id, "Activated");
      }
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void delete_customer() {
    try {
      rbac_.enforce(user_, service::Permission::DELETE_CUSTOMER,
                    "delete_customer");
      int64_t id = get_id_input("Customer ID to delete: ");
      auto opt = svc_.get(id);
      if (!opt) {
        print_warning("Not found");
        press_enter();
        return;
      }
      std::cout << Color::RED << "Delete customer: " << opt->name
                << "? (y/n): " << Color::RESET;
      char c;
      std::cin >> c;
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      if (c == 'y' || c == 'Y') {
        svc_.remove(id);
        print_success("Deleted.");
        AUDIT(user_, models::AuditAction::DELETE, "Customer", id,
              "Deleted: " + opt->name);
      }
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void list_by_tier() {
    print_header("Select Tier");
    std::cout << "  [1] Bronze\n  [2] Silver\n  [3] Gold\n  [4] Enterprise\n";
    int t = get_int_input("Tier: ", 1, 4);
    models::CustomerTier tier = (t == 1)   ? models::CustomerTier::BRONZE
                                : (t == 2) ? models::CustomerTier::SILVER
                                : (t == 3) ? models::CustomerTier::GOLD
                                           : models::CustomerTier::ENTERPRISE;
    auto customers = svc_.list_by_tier(tier);
    print_header("Tier: " + std::to_string(t) +
                 " customers: " + std::to_string(customers.size()));
    for (auto &c : customers)
      std::cout << "  " << c.id << " | " << c.name << " | " << c.email
                << " | Score: " << c.credit_score << "\n";
    press_enter();
  }

  service::CustomerService &svc_;
  service::RBACService &rbac_;
  std::string user_;
};

} // namespace billing::cli
