#pragma once
// =============================================================================
// cli_helpers.hpp — Shared CLI formatting utilities
// =============================================================================
#include <ctime>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

namespace billing::cli {

// ANSI colors
namespace Color {
inline const char *RESET = "\033[0m";
inline const char *BOLD = "\033[1m";
inline const char *RED = "\033[31m";
inline const char *GREEN = "\033[32m";
inline const char *YELLOW = "\033[33m";
inline const char *BLUE = "\033[34m";
inline const char *CYAN = "\033[36m";
inline const char *WHITE = "\033[37m";
} // namespace Color

inline void print_header(const std::string &title) {
  std::cout << "\n" << Color::CYAN << Color::BOLD;
  std::cout << "╔══════════════════════════════════════════════════════╗\n";
  std::cout << "║  " << std::left << std::setw(52) << title << "║\n";
  std::cout << "╚══════════════════════════════════════════════════════╝";
  std::cout << Color::RESET << "\n";
}

inline void print_divider() {
  std::cout << Color::BLUE
            << "──────────────────────────────────────────────────────\n"
            << Color::RESET;
}

inline void print_success(const std::string &msg) {
  std::cout << Color::GREEN << "✓ " << msg << Color::RESET << "\n";
}

inline void print_error(const std::string &msg) {
  std::cout << Color::RED << "✗ ERROR: " << msg << Color::RESET << "\n";
}

inline void print_warning(const std::string &msg) {
  std::cout << Color::YELLOW << "⚠ " << msg << Color::RESET << "\n";
}

inline void print_info(const std::string &msg) {
  std::cout << Color::CYAN << "ℹ " << msg << Color::RESET << "\n";
}

inline std::string format_currency(double amount,
                                   const std::string &currency = "USD") {
  std::ostringstream oss;
  oss << currency << " " << std::fixed << std::setprecision(2) << amount;
  return oss.str();
}

inline std::string format_time(std::time_t t) {
  if (t == 0)
    return "N/A";
  std::tm *tm = std::localtime(&t);
  char buf[32];
  std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", tm);
  return std::string(buf);
}

inline int get_int_input(const std::string &prompt, int min_val = 0,
                         int max_val = 999999) {
  int val = -1;
  while (true) {
    std::cout << Color::YELLOW << prompt << Color::RESET;
    if (std::cin >> val && val >= min_val && val <= max_val) {
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      return val;
    }
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    print_error("Invalid input. Enter a number between " +
                std::to_string(min_val) + " and " + std::to_string(max_val));
  }
}

inline double get_double_input(const std::string &prompt) {
  double val;
  while (true) {
    std::cout << Color::YELLOW << prompt << Color::RESET;
    if (std::cin >> val && val >= 0) {
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      return val;
    }
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    print_error("Invalid amount. Enter a positive number.");
  }
}

inline std::string get_string_input(const std::string &prompt) {
  std::cout << Color::YELLOW << prompt << Color::RESET;
  std::string s;
  std::getline(std::cin, s);
  return s;
}

inline int64_t get_id_input(const std::string &prompt) {
  int64_t val;
  while (true) {
    std::cout << Color::YELLOW << prompt << Color::RESET;
    if (std::cin >> val && val > 0) {
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      return val;
    }
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    print_error("Invalid ID. Enter a positive number.");
  }
}

inline void press_enter() {
  std::cout << Color::BLUE << "\nPress Enter to continue..." << Color::RESET;
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

} // namespace billing::cli
