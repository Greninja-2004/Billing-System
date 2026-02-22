#pragma once
// =============================================================================
// report_service.hpp — Reporting & Analytics Service
// Features: Aging (bucket sort), Revenue forecasting (SMA), CLV, CSV/JSON
// export
// =============================================================================
#include "../models/customer.hpp"
#include "../models/invoice.hpp"
#include "../models/payment.hpp"
#include "../repository/customer_repository.hpp"
#include "../repository/invoice_repository.hpp"
#include "../repository/payment_repository.hpp"
#include <algorithm>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <map>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace billing::service {

// Aging bucket definitions (days overdue)
struct AgingBucket {
  std::string label;
  int days_from;
  int days_to; // -1 = unlimited
  std::vector<models::Invoice> invoices;
  double total_amount = 0.0;
};

struct AgingReport {
  AgingBucket current;   // 0-30
  AgingBucket bucket_30; // 31-60
  AgingBucket bucket_60; // 61-90
  AgingBucket bucket_90; // 90+
  double grand_total_overdue = 0.0;
};

struct CLVReport {
  int64_t customer_id;
  std::string customer_name;
  double avg_monthly_revenue;
  double lifespan_months;
  double clv;
  double total_paid;
};

class ReportService {
public:
  ReportService(repository::InvoiceRepository &inv_repo,
                repository::CustomerRepository &cust_repo,
                repository::PaymentRepository &pay_repo,
                const std::string &export_dir)
      : inv_repo_(inv_repo), cust_repo_(cust_repo), pay_repo_(pay_repo),
        export_dir_(export_dir) {}

  // =========================================================================
  // Aging Report — Bucket Sort O(n)
  // =========================================================================
  AgingReport aging_report() const {
    AgingReport report;
    report.current = {"0-30 days", 0, 30};
    report.bucket_30 = {"31-60 days", 31, 60};
    report.bucket_60 = {"61-90 days", 61, 90};
    report.bucket_90 = {"90+ days", 91, -1};

    auto all = inv_repo_.find_all();
    for (auto &inv : all) {
      if (inv.status == models::InvoiceStatus::PAID ||
          inv.status == models::InvoiceStatus::CANCELLED)
        continue;

      double due = inv.amount_due();
      int days = inv.days_overdue();

      auto &bucket = (days <= 30)   ? report.current
                     : (days <= 60) ? report.bucket_30
                     : (days <= 90) ? report.bucket_60
                                    : report.bucket_90;
      bucket.invoices.push_back(inv);
      bucket.total_amount += due;
      report.grand_total_overdue += due;
    }
    return report;
  }

  // =========================================================================
  // Revenue Forecasting — Simple Moving Average (SMA-N) O(n)
  // =========================================================================
  struct MonthlyRevenue {
    std::string month;
    double revenue;
  };

  std::vector<MonthlyRevenue> monthly_revenue_history() const {
    auto payments = pay_repo_.find_all();
    std::map<std::string, double> by_month;

    for (auto &p : payments) {
      if (p.status != models::PaymentStatus::COMPLETED)
        continue;
      std::tm *t = std::localtime(&p.completed_at);
      char buf[8];
      std::strftime(buf, sizeof(buf), "%Y-%m", t);
      by_month[buf] += p.amount;
    }

    std::vector<MonthlyRevenue> result;
    for (auto &[month, rev] : by_month)
      result.push_back({month, rev});
    std::sort(result.begin(), result.end(),
              [](const MonthlyRevenue &a, const MonthlyRevenue &b) {
                return a.month < b.month;
              });
    return result;
  }

  std::vector<double> sma_forecast(int window = 3,
                                   int forecast_months = 3) const {
    auto history = monthly_revenue_history();
    if (history.empty())
      return {};

    std::vector<double> revenues;
    for (auto &m : history)
      revenues.push_back(m.revenue);

    std::vector<double> forecasts;
    for (int i = 0; i < forecast_months; ++i) {
      int start = std::max(0, static_cast<int>(revenues.size()) - window);
      double sum = 0;
      int count = 0;
      for (int j = start; j < static_cast<int>(revenues.size()); ++j) {
        sum += revenues[j];
        count++;
      }
      double forecast = count > 0 ? sum / count : 0.0;
      forecasts.push_back(forecast);
      revenues.push_back(forecast); // use forecast in next window
    }
    return forecasts;
  }

  // =========================================================================
  // Customer Lifetime Value (CLV) — O(n payments per customer)
  // CLV = avg_monthly_revenue * lifespan_months
  // =========================================================================
  std::vector<CLVReport> customer_clv_report() const {
    auto customers = cust_repo_.find_all();
    std::vector<CLVReport> result;

    for (auto &cust : customers) {
      auto payments = pay_repo_.find_by_customer(cust.id);
      double total_paid = 0.0;
      for (auto &p : payments)
        if (p.status == models::PaymentStatus::COMPLETED)
          total_paid += p.amount;

      double months = std::max(1.0, cust.lifetime_months());
      double avg_monthly = total_paid / months;
      double clv = avg_monthly * 24.0; // assume 24-month lifespan

      result.push_back(
          {cust.id, cust.name, avg_monthly, months, clv, total_paid});
    }

    // Sort by CLV descending
    std::sort(
        result.begin(), result.end(),
        [](const CLVReport &a, const CLVReport &b) { return a.clv > b.clv; });
    return result;
  }

  // =========================================================================
  // Export to CSV
  // =========================================================================
  std::string export_aging_csv(const AgingReport &report) const {
    std::string path = export_dir_ + "/aging_report.csv";
    std::ofstream f(path);
    if (!f.is_open())
      throw std::runtime_error("Cannot write to: " + path);

    f << "Invoice ID,Customer ID,Invoice#,Status,Total,Amount Due,Days "
         "Overdue,Bucket\n";
    auto write_bucket = [&](const AgingBucket &b) {
      for (auto &inv : b.invoices) {
        f << inv.id << "," << inv.customer_id << "," << inv.invoice_number
          << "," << models::invoice_status_to_string(inv.status) << ","
          << std::fixed << std::setprecision(2) << inv.total_amount << ","
          << inv.amount_due() << "," << inv.days_overdue() << "," << b.label
          << "\n";
      }
    };
    write_bucket(report.current);
    write_bucket(report.bucket_30);
    write_bucket(report.bucket_60);
    write_bucket(report.bucket_90);
    f << ",,TOTAL,,,,," << report.grand_total_overdue << "\n";
    return path;
  }

  std::string export_clv_csv(const std::vector<CLVReport> &reports) const {
    std::string path = export_dir_ + "/clv_report.csv";
    std::ofstream f(path);
    if (!f.is_open())
      throw std::runtime_error("Cannot write to: " + path);

    f << "Customer ID,Name,Total Paid,Months Active,Avg Monthly Revenue,CLV "
         "(24m)\n";
    for (auto &r : reports) {
      f << r.customer_id << "," << r.customer_name << "," << std::fixed
        << std::setprecision(2) << r.total_paid << "," << r.lifespan_months
        << "," << r.avg_monthly_revenue << "," << r.clv << "\n";
    }
    return path;
  }

  std::string export_revenue_json(const std::vector<MonthlyRevenue> &history,
                                  const std::vector<double> &forecast) const {
    std::string path = export_dir_ + "/revenue_report.json";
    std::ofstream f(path);
    if (!f.is_open())
      throw std::runtime_error("Cannot write to: " + path);

    f << "{\n  \"history\": [\n";
    for (std::size_t i = 0; i < history.size(); ++i) {
      f << "    {\"month\": \"" << history[i].month
        << "\", \"revenue\": " << std::fixed << std::setprecision(2)
        << history[i].revenue << "}";
      if (i + 1 < history.size())
        f << ",";
      f << "\n";
    }
    f << "  ],\n  \"forecast\": [";
    for (std::size_t i = 0; i < forecast.size(); ++i) {
      f << std::fixed << std::setprecision(2) << forecast[i];
      if (i + 1 < forecast.size())
        f << ", ";
    }
    f << "]\n}\n";
    return path;
  }

  // Summary stats
  struct Summary {
    std::size_t total_customers;
    std::size_t total_invoices;
    std::size_t total_payments;
    double total_revenue;
    double total_outstanding;
    std::size_t overdue_count;
  };

  Summary generate_summary() const {
    Summary s{};
    s.total_customers = cust_repo_.count();
    s.total_invoices = inv_repo_.count();
    s.total_payments = pay_repo_.count();

    for (auto &p : pay_repo_.find_all())
      if (p.status == models::PaymentStatus::COMPLETED)
        s.total_revenue += p.amount;
    for (auto &inv : inv_repo_.find_all()) {
      s.total_outstanding += inv.amount_due();
      if (inv.is_overdue())
        s.overdue_count++;
    }
    return s;
  }

private:
  repository::InvoiceRepository &inv_repo_;
  repository::CustomerRepository &cust_repo_;
  repository::PaymentRepository &pay_repo_;
  std::string export_dir_;
};

} // namespace billing::service
