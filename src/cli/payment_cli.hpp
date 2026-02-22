#pragma once
// =============================================================================
// payment_cli.hpp — Payment Processing CLI Module
// =============================================================================
#include "../repository/payment_repository.hpp"
#include "../service/audit_service.hpp"
#include "../service/fraud_detector.hpp"
#include "../service/payment_processor.hpp"
#include "../service/rbac_service.hpp"
#include "cli_helpers.hpp"
#include <iomanip>
#include <iostream>

namespace billing::cli {

class PaymentCLI {
public:
  PaymentCLI(service::PaymentProcessor &processor,
             service::FraudDetector &fraud,
             repository::PaymentRepository &pay_repo,
             service::RBACService &rbac, const std::string &current_user)
      : processor_(processor), fraud_(fraud), pay_repo_(pay_repo), rbac_(rbac),
        user_(current_user) {}

  void run() {
    while (true) {
      print_header("Payment Processing");
      std::cout << "  [1] Process Payment\n"
                << "  [2] Process Refund\n"
                << "  [3] Payment History (by Customer)\n"
                << "  [4] Payment Details (by ID)\n"
                << "  [5] All Payments\n"
                << "  [6] Fraud Check (manual)\n"
                << "  [0] Back\n";
      print_divider();
      int choice = get_int_input("Select option: ", 0, 6);
      switch (choice) {
      case 0:
        return;
      case 1:
        process_payment();
        break;
      case 2:
        process_refund();
        break;
      case 3:
        payment_history();
        break;
      case 4:
        payment_detail();
        break;
      case 5:
        all_payments();
        break;
      case 6:
        fraud_check();
        break;
      }
    }
  }

private:
  void print_payment(const models::Payment &p) {
    auto color = p.status == models::PaymentStatus::COMPLETED ? Color::GREEN
                 : p.status == models::PaymentStatus::FAILED  ? Color::RED
                                                              : Color::YELLOW;
    std::cout << Color::BOLD << "Payment ID: " << Color::RESET << p.id << "\n"
              << "  Invoice ID:  " << p.invoice_id << "\n"
              << "  Customer ID: " << p.customer_id << "\n"
              << "  Method:      " << models::payment_method_to_string(p.method)
              << "\n"
              << "  Status:      " << color
              << models::payment_status_to_string(p.status) << Color::RESET
              << "\n"
              << "  Amount:      " << format_currency(p.amount) << "\n"
              << "  Refunded:    " << format_currency(p.refund_amount) << "\n"
              << "  Gateway Ref: " << p.gateway_ref << "\n"
              << "  Retries:     " << p.retry_count << "\n"
              << "  Fraud Flag:  " << (p.fraud_flagged ? "YES ⚠" : "No") << "\n"
              << "  Created:     " << format_time(p.created_at) << "\n";
  }

  void process_payment() {
    try {
      rbac_.enforce(user_, service::Permission::PROCESS_PAYMENT);
      print_header("Process Payment");

      int64_t inv_id = get_id_input("Invoice ID: ");
      int64_t cust_id = get_id_input("Customer ID: ");
      double amount = get_double_input("Payment Amount ($): ");

      // Run fraud check first
      auto signal = fraud_.check(cust_id, amount);
      if (signal.flagged) {
        print_warning("⚠ FRAUD ALERT: " + signal.reason);
        print_warning("Risk Score: " + std::to_string(signal.risk_score));
        std::cout << "Continue anyway? (y/n): ";
        char c;
        std::cin >> c;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (c != 'y' && c != 'Y') {
          print_info("Payment cancelled.");
          press_enter();
          return;
        }
      }

      std::cout
          << "  Gateway: [1] Credit Card  [2] Bank Transfer  [3] Wallet: ";
      int gw = get_int_input("", 1, 3);
      models::PaymentMethod method =
          (gw == 1)   ? models::PaymentMethod::CREDIT_CARD
          : (gw == 2) ? models::PaymentMethod::BANK_TRANSFER
                      : models::PaymentMethod::WALLET;
      auto notes = get_string_input("Notes (optional): ");

      print_info("Processing payment" +
                 std::string(signal.flagged ? " (fraud-flagged)" : "") + "...");
      auto result =
          processor_.process_payment(inv_id, cust_id, amount, method, notes);

      if (result.success) {
        print_success(result.message);
        if (result.credit_balance > 0)
          print_info("Overpayment credit: " +
                     format_currency(result.credit_balance));
      } else {
        print_error("Payment failed: " + result.message);
      }
      print_payment(result.payment);
      AUDIT(user_, models::AuditAction::PAYMENT, "Invoice", inv_id,
            "Payment $" + std::to_string(amount) + " via " +
                models::payment_method_to_string(method));
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void process_refund() {
    try {
      rbac_.enforce(user_, service::Permission::ISSUE_REFUND);
      print_header("Process Refund");
      int64_t pay_id = get_id_input("Payment ID to refund: ");
      double amount = get_double_input("Refund amount ($): ");
      auto reason = get_string_input("Reason: ");

      auto result = processor_.process_refund(pay_id, amount, reason);
      if (result.success) {
        print_success(result.message);
        AUDIT(user_, models::AuditAction::REFUND, "Payment", pay_id,
              "Refund $" + std::to_string(amount) + ": " + reason);
      } else {
        print_error(result.message);
      }
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void payment_history() {
    try {
      rbac_.enforce(user_, service::Permission::READ_INVOICE);
      int64_t cid = get_id_input("Customer ID: ");
      auto payments = pay_repo_.find_by_customer(cid);
      print_header("Payment History (" + std::to_string(payments.size()) +
                   " records)");
      double total = 0;
      for (auto &p : payments) {
        std::cout << "  " << format_time(p.created_at) << " | "
                  << models::payment_method_to_string(p.method) << " | "
                  << models::payment_status_to_string(p.status) << " | $"
                  << std::fixed << std::setprecision(2) << p.amount
                  << (p.fraud_flagged ? " [FRAUD]" : "") << "\n";
        if (p.status == models::PaymentStatus::COMPLETED)
          total += p.amount;
      }
      print_divider();
      std::cout << Color::BOLD << "Total Completed: " << format_currency(total)
                << Color::RESET << "\n";
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void payment_detail() {
    try {
      rbac_.enforce(user_, service::Permission::READ_INVOICE);
      int64_t pid = get_id_input("Payment ID: ");
      auto opt = pay_repo_.find_by_id(pid);
      if (opt)
        print_payment(*opt);
      else
        print_warning("Payment not found.");
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void all_payments() {
    try {
      rbac_.enforce(user_, service::Permission::READ_INVOICE);
      auto payments = pay_repo_.find_all();
      print_header("All Payments (" + std::to_string(payments.size()) + ")");
      std::cout << std::left << std::setw(20) << "ID" << std::setw(20)
                << "Invoice" << std::setw(16) << "Method" << std::setw(14)
                << "Status" << std::setw(12) << "Amount\n";
      print_divider();
      for (auto &p : payments) {
        std::cout << std::left << std::setw(20) << p.id << std::setw(20)
                  << p.invoice_id << std::setw(16)
                  << models::payment_method_to_string(p.method) << std::setw(14)
                  << models::payment_status_to_string(p.status) << "$"
                  << std::fixed << std::setprecision(2) << p.amount << "\n";
      }
      press_enter();
    } catch (const std::exception &e) {
      print_error(e.what());
      press_enter();
    }
  }

  void fraud_check() {
    int64_t cid = get_id_input("Customer ID: ");
    double amt = get_double_input("Transaction Amount ($): ");
    auto signal = fraud_.check(cid, amt);
    print_header("Fraud Analysis Result");
    std::cout << "  Flagged:    "
              << (signal.flagged ? std::string(Color::RED) + "YES"
                                 : std::string(Color::GREEN) + "NO")
              << Color::RESET << "\n"
              << "  Risk Score: " << std::fixed << std::setprecision(2)
              << signal.risk_score << "\n"
              << "  Reasons:    "
              << (signal.reason.empty() ? "None" : signal.reason) << "\n"
              << "  Tx count in window: " << fraud_.transaction_count(cid)
              << "\n";
    press_enter();
  }

  service::PaymentProcessor &processor_;
  service::FraudDetector &fraud_;
  repository::PaymentRepository &pay_repo_;
  service::RBACService &rbac_;
  std::string user_;
};

} // namespace billing::cli
