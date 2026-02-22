#pragma once
// =============================================================================
// invoice_cli.hpp — Invoice & Billing CLI Module
// =============================================================================
#include "../repository/invoice_repository.hpp"
#include "../service/audit_service.hpp"
#include "../service/billing_engine.hpp"
#include "../service/customer_service.hpp"
#include "../service/rbac_service.hpp"
#include "cli_helpers.hpp"
#include <iomanip>
#include <iostream>

namespace billing::cli {

class InvoiceCLI {
public:
  InvoiceCLI(service::BillingEngine &engine, service::CustomerService &cust_svc,
             repository::InvoiceRepository &inv_repo,
             service::RBACService &rbac, const std::string &current_user)
      : engine_(engine), cust_svc_(cust_svc), inv_repo_(inv_repo), rbac_(rbac),
        user_(current_user) {}

  void run() {
    while (true) {
      print_header("Invoice & Billing Engine");
      std::cout << "  [1] List All Invoices\n"
                << "  [2] View Invoice by ID\n"
                << "  [3] Invoices for Customer\n"
                << "  [4] Create One-Time Invoice\n"
                << "  [5] Create Recurring Invoice\n"
                << "  [6] Create Prorated Invoice\n"
                << "  [7] Generate Next Recurring Invoice\n"
                << "  [8] List Overdue Invoices\n"
                << "  [9] Scan & Flag All Overdue\n"
                << " [10] Next Invoice Due (Scheduler)\n"
                << "  [0] Back\n";
      print_divider();
      int choice = get_int_input("Select option: ", 0, 10);
      switch (choice) {
      case 0:
        return;
      case 1:
        list_all();
        break;
      case 2:
        view_by_id();
        break;
      case 3:
        by_customer();
        break;
      case 4:
        create_invoice(models::InvoiceType::ONE_TIME);
        break;
      case 5:
        create_invoice(models::InvoiceType::RECURRING);
        break;
      case 6:
        create_invoice(models::InvoiceType::PRORATED);
        break;
      case 7:
        gen_next_recurring();
        break;
      case 8:
        list_overdue();
        break;
      case 9:
        flag_overdue();
        break;
      case 10:
        next_due();
        break;
      }
    }
  }

private:
  void print_invoice(const models::Invoice &inv) {
    print_divider();
    auto status_color = [&]() -> std::string {
      switch (inv.status) {
      case models::InvoiceStatus::PAID:
        return Color::GREEN;
      case models::InvoiceStatus::OVERDUE:
        return Color::RED;
      case models::InvoiceStatus::PENDING:
        return Color::YELLOW;
      default:
        return Color::WHITE;
      }
    };
    std::cout << Color::BOLD << "Invoice: " << Color::RESET
              << inv.invoice_number << "\n"
              << "  ID:           " << inv.id << "\n"
              << "  Customer ID:  " << inv.customer_id << "\n"
              << "  Type:         " << models::invoice_type_to_string(inv.type)
              << "\n"
              << "  Status:       " << status_color()
              << models::invoice_status_to_string(inv.status) << Color::RESET
              << "\n"
              << "  Subtotal:     " << format_currency(inv.subtotal) << "\n"
              << "  Discount:     " << format_currency(inv.discount_amount)
              << "\n"
              << "  Tax:          " << format_currency(inv.tax_amount) << "\n"
              << "  Total:        " << Color::BOLD
              << format_currency(inv.total_amount) << Color::RESET << "\n"
              << "  Paid:         " << format_currency(inv.amount_paid) << "\n"
              << "  Due:          " << Color::RED
              << format_currency(inv.amount_due()) << Color::RESET << "\n"
              << "  Issue Date:   " << format_time(inv.issue_date) << "\n"
              << "  Due Date:     " << format_time(inv.due_date) << "\n";
    if (!inv.line_items.empty()) {
      std::cout << "  Line Items:\n";
      for (auto &li : inv.line_items)
        std::cout << "    - " << li.description << " x" << li.quantity << " @ $"
                  << std::fixed << std::setprecision(2) << li.unit_price
                  << " = $" << li.total() << "\n";
    }
  }

  void list_all() {
    try {
      rbac_.enforce(user_, service::Permission::READ_INVOICE);
      auto invs = inv_repo_.find_all();
      print_header("All Invoices (" + std::to_string(invs.size()) + ")");
      std::cout << std::left << std::setw(20) << "ID" << std::setw(16)
                << "Invoice#" << std::setw(20) << "Customer ID" << std::setw(14)
                << "Type" << std::setw(16) << "Status" << std::setw(12)
                << "Total" << std::setw(12) << "Due\n";
      print_divider();
      for (auto &inv : invs) {
        std::cout << std::left << std::setw(20) << inv.id << std::setw(16)
                  << inv.invoice_number << std::setw(20) << inv.customer_id
                  << std::setw(14) << models::invoice_type_to_string(inv.type)
                  << std::setw(16)
                  << models::invoice_status_to_string(inv.status)
                  << std::setw(12) << std::fixed << std::setprecision(2)
                  << inv.total_amount << std::setw(12) << inv.amount_due()
                  << "\n";
      }
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void view_by_id() {
    try {
      rbac_.enforce(user_, service::Permission::READ_INVOICE);
      int64_t id = get_id_input("Invoice ID: ");
      auto opt = inv_repo_.find_by_id(id);
      if (opt)
        print_invoice(*opt);
      else
        print_warning("Invoice not found.");
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void by_customer() {
    try {
      rbac_.enforce(user_, service::Permission::READ_INVOICE);
      int64_t cid = get_id_input("Customer ID: ");
      auto invs = inv_repo_.find_by_customer(cid);
      print_header("Invoices for Customer " + std::to_string(cid) + " (" +
                   std::to_string(invs.size()) + ")");
      for (auto &inv : invs)
        print_invoice(inv);
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void create_invoice(models::InvoiceType type) {
    try {
      rbac_.enforce(user_, service::Permission::WRITE_INVOICE);
      print_header("Create " + models::invoice_type_to_string(type) +
                   " Invoice");

      service::InvoiceRequest req;
      req.type = type;
      req.customer_id = get_id_input("Customer ID: ");
      req.currency = "USD";
      req.due_days = get_int_input("Due in (days): ", 1, 365);
      req.notes = get_string_input("Notes (optional): ");

      if (type == models::InvoiceType::RECURRING) {
        std::cout << "  Period: [1] Monthly [2] Weekly [3] Daily [4] Yearly: ";
        int p = get_int_input("", 1, 4);
        req.period = (p == 1)   ? models::RecurringPeriod::MONTHLY
                     : (p == 2) ? models::RecurringPeriod::WEEKLY
                     : (p == 3) ? models::RecurringPeriod::DAILY
                                : models::RecurringPeriod::YEARLY;
      }
      if (type == models::InvoiceType::PRORATED) {
        int days_used = get_int_input("Days used in this period: ", 1, 31);
        std::time_t now = std::time(nullptr);
        req.period_start = now - days_used * 86400;
        req.period_end = now;
      }

      // Collect line items
      int num = get_int_input("Number of line items: ", 1, 20);
      for (int i = 0; i < num; ++i) {
        models::LineItem li;
        li.description = get_string_input("  Item " + std::to_string(i + 1) +
                                          " description: ");
        li.quantity = get_int_input("  Quantity: ", 1, 10000);
        li.unit_price = get_double_input("  Unit price ($): ");
        req.line_items.push_back(li);
      }

      auto inv = engine_.create_invoice(req);
      print_success("Invoice created: " + inv.invoice_number);
      print_invoice(inv);
      AUDIT(user_, models::AuditAction::CREATE, "Invoice", inv.id,
            "Created " + models::invoice_type_to_string(type) + " invoice " +
                inv.invoice_number);
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void gen_next_recurring() {
    try {
      rbac_.enforce(user_, service::Permission::WRITE_INVOICE);
      int64_t id = get_id_input("Parent Invoice ID: ");
      auto opt = inv_repo_.find_by_id(id);
      if (!opt) {
        print_warning("Invoice not found");
        press_enter();
        return;
      }
      auto next = engine_.generate_next_recurring(*opt);
      if (next) {
        print_success("Next recurring invoice: " + next->invoice_number);
        print_invoice(*next);
        AUDIT(user_, models::AuditAction::CREATE, "Invoice", next->id,
              "Generated next recurring");
      } else {
        print_warning("This invoice does not have recurring schedule.");
      }
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void list_overdue() {
    auto invs = inv_repo_.find_overdue();
    print_header("Overdue Invoices (" + std::to_string(invs.size()) + ")");
    for (auto &inv : invs) {
      std::cout << Color::RED << "  " << inv.invoice_number
                << " | Days overdue: " << inv.days_overdue() << " | Due: $"
                << std::fixed << std::setprecision(2) << inv.amount_due()
                << Color::RESET << "\n";
    }
    press_enter();
  }

  void flag_overdue() {
    int count = engine_.flag_overdue();
    print_info(std::to_string(count) + " invoices flagged as overdue.");
    press_enter();
  }

  void next_due() {
    auto opt = engine_.next_due();
    if (opt) {
      print_header("Next Due Invoice (from scheduler)");
      print_invoice(*opt);
    } else {
      print_info("No invoices in scheduler queue.");
    }
    press_enter();
  }

  service::BillingEngine &engine_;
  service::CustomerService &cust_svc_;
  repository::InvoiceRepository &inv_repo_;
  service::RBACService &rbac_;
  std::string user_;
};

} // namespace billing::cli
