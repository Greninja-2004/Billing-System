// =============================================================================
// main.cpp — Billing System Entry Point
// =============================================================================
#include "cli/admin_cli.hpp"
#include "cli/cli_helpers.hpp"
#include "cli/customer_cli.hpp"
#include "cli/invoice_cli.hpp"
#include "cli/payment_cli.hpp"
#include "cli/report_cli.hpp"
#include "data/sample_loader.hpp"
#include "repository/customer_repository.hpp"
#include "repository/invoice_repository.hpp"
#include "repository/payment_repository.hpp"
#include "service/audit_service.hpp"
#include "service/billing_engine.hpp"
#include "service/customer_service.hpp"
#include "service/discount_engine.hpp"
#include "service/fraud_detector.hpp"
#include "service/notification_service.hpp"
#include "service/payment_processor.hpp"
#include "service/rbac_service.hpp"
#include "service/report_service.hpp"
#include "service/tax_engine.hpp"
#include <filesystem>
#include <iostream>
#include <limits>
#include <string>

using namespace billing;

// ---------------------------------------------------------------------------
// Application context — holds all singletons
// ---------------------------------------------------------------------------
struct AppContext {
  // Data directories
  std::string data_dir = "data";
  std::string export_dir = "exports";

  // Repositories
  repository::CustomerRepository cust_repo{data_dir};
  repository::InvoiceRepository inv_repo{data_dir};
  repository::PaymentRepository pay_repo{data_dir};

  // Services
  service::DiscountEngine discount;
  service::TaxEngine tax;
  service::CustomerService cust_svc{cust_repo};
  service::BillingEngine billing{inv_repo, cust_repo, discount, tax};
  service::PaymentProcessor payment{inv_repo, pay_repo};
  service::FraudDetector fraud{60, 10, 5000.0};
  service::ReportService reports{inv_repo, cust_repo, pay_repo, export_dir};
  service::NotificationService notif;
  service::RBACService rbac;
};

// ---------------------------------------------------------------------------
// Login screen
// ---------------------------------------------------------------------------
std::string login_screen(service::RBACService &rbac) {
  cli::print_header("Welcome to Billing System Pro");
  std::cout << "\n  Default accounts:\n"
            << "    admin   / admin123\n"
            << "    manager / manager123\n"
            << "    agent1  / agent123\n"
            << "    viewer  / readonly\n\n";

  for (int attempts = 0; attempts < 3; ++attempts) {
    std::string uid = cli::get_string_input("Username: ");
    std::string pwd = cli::get_string_input("Password: ");
    auto user = rbac.login(uid, pwd);
    if (user) {
      cli::print_success("Welcome, " + user->name + "! Role: " + user->role);
      service::AuditService::instance().log(uid, models::AuditAction::LOGIN,
                                            "Session", 0,
                                            "User logged in: " + uid);
      return uid;
    }
    cli::print_error("Invalid credentials. " + std::to_string(2 - attempts) +
                     " attempts remaining.");
  }
  cli::print_error("Too many failed attempts. Exiting.");
  std::exit(1);
}

// ---------------------------------------------------------------------------
// Main menu
// ---------------------------------------------------------------------------
void main_menu(AppContext &ctx, const std::string &current_user) {
  // Register notification service as observer
  ctx.billing.add_observer(&ctx.notif);

  cli::CustomerCLI customer_cli(ctx.cust_svc, ctx.rbac, current_user);
  cli::InvoiceCLI invoice_cli(ctx.billing, ctx.cust_svc, ctx.inv_repo, ctx.rbac,
                              current_user);
  cli::PaymentCLI payment_cli(ctx.payment, ctx.fraud, ctx.pay_repo, ctx.rbac,
                              current_user);
  cli::ReportCLI report_cli(ctx.reports, ctx.rbac, current_user);
  cli::AdminCLI admin_cli(ctx.rbac, ctx.notif, current_user);

  while (true) {
    std::cout << "\n";
    cli::print_header("Billing System Pro — Main Menu");
    std::cout << "  [1] Customer Management\n"
              << "  [2] Invoice & Billing\n"
              << "  [3] Payment Processing\n"
              << "  [4] Reports & Analytics\n"
              << "  [5] Security & Administration\n"
              << "  [6] Load Sample Dataset (100 customers, 500 invoices)\n"
              << "  [7] System Status\n"
              << "  [0] Logout & Exit\n";
    cli::print_divider();
    std::cout << cli::Color::CYAN << "  Logged in as: " << current_user
              << " | Notifications queued: " << ctx.notif.queue_size()
              << cli::Color::RESET << "\n";

    int choice = cli::get_int_input("Select option: ", 0, 7);
    switch (choice) {
    case 0:
      service::AuditService::instance().log(
          current_user, models::AuditAction::LOGOUT, "Session", 0, "Logout");
      cli::print_info("Goodbye!");
      return;
    case 1:
      customer_cli.run();
      break;
    case 2:
      invoice_cli.run();
      break;
    case 3:
      payment_cli.run();
      break;
    case 4:
      report_cli.run();
      break;
    case 5:
      admin_cli.run();
      break;
    case 6: {
      if (ctx.cust_repo.count() > 0) {
        cli::print_warning(
            "Data already loaded (" + std::to_string(ctx.cust_repo.count()) +
            " customers, " + std::to_string(ctx.inv_repo.count()) +
            " invoices). Skipping.");
      } else {
        data::SampleLoader loader(ctx.cust_svc, ctx.billing);
        auto [c, i] = loader.load(100, 500);
        cli::print_success("Sample data loaded: " + std::to_string(c) +
                           " customers, " + std::to_string(i) + " invoices.");
      }
      cli::press_enter();
      break;
    }
    case 7: {
      auto s = ctx.reports.generate_summary();
      cli::print_header("System Status");
      std::cout << "  Customers:     " << s.total_customers << "\n"
                << "  Invoices:      " << s.total_invoices << "\n"
                << "  Payments:      " << s.total_payments << "\n"
                << "  Revenue:       " << cli::format_currency(s.total_revenue)
                << "\n"
                << "  Outstanding:   "
                << cli::format_currency(s.total_outstanding) << "\n"
                << "  Overdue:       " << s.overdue_count << "\n"
                << "  Cache hit rate:" << std::fixed << std::setprecision(1)
                << (ctx.cust_repo.cache_hit_rate() * 100.0) << "%\n"
                << "  Notifications queued: " << ctx.notif.queue_size() << "\n"
                << "  Audit entries: "
                << service::AuditService::instance().count() << "\n";
      cli::press_enter();
      break;
    }
    }
  }
}

// ---------------------------------------------------------------------------
// main()
// ---------------------------------------------------------------------------
int main(int argc, char *argv[]) {
  // Ensure data & export directories exist
  std::filesystem::create_directories("data");
  std::filesystem::create_directories("exports");

  // Initialize audit service with correct log path
  service::AuditService::instance("data/audit.log");

  std::cout << cli::Color::CYAN << cli::Color::BOLD
            << "\n╔══════════════════════════════════════════════════════╗\n"
            << "║         Billing System Pro  v1.0                    ║\n"
            << "║  Industry-Level C++17 Billing & Invoice Platform    ║\n"
            << "╚══════════════════════════════════════════════════════╝\n"
            << cli::Color::RESET;

  // Parse optional --demo flag for auto-loading sample data
  bool auto_load = false;
  for (int i = 1; i < argc; ++i)
    if (std::string(argv[i]) == "--demo")
      auto_load = true;

  try {
    AppContext ctx;

    // Auto-load demo data if flag present and no data exists
    if (auto_load && ctx.cust_repo.count() == 0) {
      std::cout << "Auto-loading sample dataset...\n";
      data::SampleLoader loader(ctx.cust_svc, ctx.billing);
      loader.load(100, 500);
    }

    std::string user = login_screen(ctx.rbac);
    main_menu(ctx, user);

  } catch (const std::exception &e) {
    std::cerr << cli::Color::RED << "Fatal error: " << e.what()
              << cli::Color::RESET << "\n";
    return 1;
  }
  return 0;
}
