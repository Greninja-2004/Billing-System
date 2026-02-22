#pragma once
// =============================================================================
// report_cli.hpp — Reporting & Analytics CLI Module
// =============================================================================
#include "../service/audit_service.hpp"
#include "../service/rbac_service.hpp"
#include "../service/report_service.hpp"
#include "cli_helpers.hpp"
#include <iomanip>
#include <iostream>

namespace billing::cli {

class ReportCLI {
public:
  ReportCLI(service::ReportService &svc, service::RBACService &rbac,
            const std::string &current_user)
      : svc_(svc), rbac_(rbac), user_(current_user) {}

  void run() {
    while (true) {
      print_header("Reports & Analytics");
      std::cout << "  [1] Dashboard Summary\n"
                << "  [2] Aging Report\n"
                << "  [3] Revenue History & Forecast\n"
                << "  [4] Customer Lifetime Value (CLV)\n"
                << "  [5] Export Aging Report → CSV\n"
                << "  [6] Export CLV Report → CSV\n"
                << "  [7] Export Revenue → JSON\n"
                << "  [0] Back\n";
      print_divider();
      int choice = get_int_input("Select option: ", 0, 7);
      switch (choice) {
      case 0:
        return;
      case 1:
        dashboard();
        break;
      case 2:
        aging_report();
        break;
      case 3:
        revenue_forecast();
        break;
      case 4:
        clv_report();
        break;
      case 5:
        export_aging_csv();
        break;
      case 6:
        export_clv_csv();
        break;
      case 7:
        export_revenue_json();
        break;
      }
    }
  }

private:
  void dashboard() {
    try {
      rbac_.enforce(user_, service::Permission::VIEW_REPORTS);
      auto s = svc_.generate_summary();
      print_header("System Dashboard");
      std::cout << Color::BOLD << "  Customers:       " << Color::RESET
                << s.total_customers << "\n"
                << Color::BOLD << "  Invoices:        " << Color::RESET
                << s.total_invoices << "\n"
                << Color::BOLD << "  Payments:        " << Color::RESET
                << s.total_payments << "\n"
                << Color::GREEN << Color::BOLD
                << "  Total Revenue:   " << Color::RESET
                << format_currency(s.total_revenue) << "\n"
                << Color::YELLOW << Color::BOLD
                << "  Total Outstanding: " << Color::RESET
                << format_currency(s.total_outstanding) << "\n"
                << Color::RED << Color::BOLD
                << "  Overdue Invoices:" << Color::RESET << " "
                << s.overdue_count << "\n";
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void aging_report() {
    try {
      rbac_.enforce(user_, service::Permission::VIEW_REPORTS);
      auto report = svc_.aging_report();
      print_header("Aging Report — Outstanding Receivables");

      auto print_bucket = [](const service::AgingBucket &b) {
        std::cout << Color::BOLD << "\n  " << b.label << " ("
                  << b.invoices.size() << " invoices): " << Color::RESET
                  << format_currency(b.total_amount) << "\n";
        for (auto &inv : b.invoices)
          std::cout << "    " << inv.invoice_number
                    << " | Customer: " << inv.customer_id
                    << " | Overdue: " << inv.days_overdue() << " days"
                    << " | Due: $" << std::fixed << std::setprecision(2)
                    << inv.amount_due() << "\n";
      };

      print_bucket(report.current);
      print_bucket(report.bucket_30);
      print_bucket(report.bucket_60);
      print_bucket(report.bucket_90);
      print_divider();
      std::cout << Color::RED << Color::BOLD << "  GRAND TOTAL OVERDUE: "
                << format_currency(report.grand_total_overdue) << Color::RESET
                << "\n";
      AUDIT(user_, models::AuditAction::READ, "Report", 0,
            "Viewed aging report");
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void revenue_forecast() {
    try {
      rbac_.enforce(user_, service::Permission::VIEW_REPORTS);
      int window = get_int_input("SMA window (months, 1-12): ", 1, 12);
      auto history = svc_.monthly_revenue_history();
      auto forecast = svc_.sma_forecast(window, 3);
      print_header("Revenue History & Forecast (SMA-" + std::to_string(window) +
                   ")");
      if (history.empty()) {
        print_warning("No payment history yet.");
      } else {
        std::cout << Color::BOLD << "  Historical Revenue:\n" << Color::RESET;
        for (auto &m : history)
          std::cout << "    " << m.month << ": " << format_currency(m.revenue)
                    << "\n";
      }
      print_divider();
      std::cout << Color::BOLD << "  3-Month Forecast:\n" << Color::RESET;
      for (std::size_t i = 0; i < forecast.size(); ++i)
        std::cout << "    Month +" << (i + 1) << ": " << Color::GREEN
                  << format_currency(forecast[i]) << Color::RESET << "\n";
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void clv_report() {
    try {
      rbac_.enforce(user_, service::Permission::VIEW_REPORTS);
      auto reports = svc_.customer_clv_report();
      print_header("Customer Lifetime Value (Top 20)");
      std::cout << std::left << std::setw(20) << "Customer ID" << std::setw(25)
                << "Name" << std::setw(15) << "Total Paid" << std::setw(12)
                << "Avg/Month" << std::setw(15) << "CLV (24m)\n";
      print_divider();
      int n = std::min(20, static_cast<int>(reports.size()));
      for (int i = 0; i < n; ++i) {
        auto &r = reports[i];
        std::cout << std::left << std::setw(20) << r.customer_id
                  << std::setw(25) << r.customer_name.substr(0, 23) << "$"
                  << std::setw(14) << std::fixed << std::setprecision(2)
                  << r.total_paid << "$" << std::setw(11)
                  << r.avg_monthly_revenue << Color::GREEN << "$" << r.clv
                  << Color::RESET << "\n";
      }
      AUDIT(user_, models::AuditAction::READ, "Report", 0, "Viewed CLV report");
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void export_aging_csv() {
    try {
      rbac_.enforce(user_, service::Permission::EXPORT_DATA);
      auto report = svc_.aging_report();
      auto path = svc_.export_aging_csv(report);
      print_success("Aging report exported to: " + path);
      AUDIT(user_, models::AuditAction::EXPORT, "Report", 0,
            "Exported aging CSV");
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void export_clv_csv() {
    try {
      rbac_.enforce(user_, service::Permission::EXPORT_DATA);
      auto reports = svc_.customer_clv_report();
      auto path = svc_.export_clv_csv(reports);
      print_success("CLV report exported to: " + path);
      AUDIT(user_, models::AuditAction::EXPORT, "Report", 0,
            "Exported CLV CSV");
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void export_revenue_json() {
    try {
      rbac_.enforce(user_, service::Permission::EXPORT_DATA);
      auto history = svc_.monthly_revenue_history();
      int w = get_int_input("SMA window (months): ", 1, 12);
      auto forecast = svc_.sma_forecast(w, 3);
      auto path = svc_.export_revenue_json(history, forecast);
      print_success("Revenue report exported to: " + path);
      AUDIT(user_, models::AuditAction::EXPORT, "Report", 0,
            "Exported revenue JSON");
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  service::ReportService &svc_;
  service::RBACService &rbac_;
  std::string user_;
};

} // namespace billing::cli
