# Billing System Pro

A complete, industry-level **C++17 Billing & Invoice Management System** demonstrating advanced data structures, algorithm design patterns, multi-threading, and layered architecture.

---

## Quick Start

```bash
# Build everything
make all

# Run the billing system
make run

# Run with auto-loaded demo data (100 customers, 500 invoices)
make demo

# Run unit tests
make run_tests

# Clean build artifacts
make clean
```

---

## Architecture

```
┌─────────────────────────────────────────────┐
│             Presentation Layer              │
│   CustomerCLI · InvoiceCLI · PaymentCLI    │
│       ReportCLI · AdminCLI                 │
├─────────────────────────────────────────────┤
│              Service Layer                  │
│  BillingEngine · PaymentProcessor          │
│  DiscountEngine · TaxEngine                │
│  FraudDetector · NotificationService       │
│  ReportService · AuditService · RBAC       │
├─────────────────────────────────────────────┤
│            Repository Layer                 │
│  CustomerRepository · InvoiceRepository    │
│  PaymentRepository (B+Tree Index, LRU)     │
├─────────────────────────────────────────────┤
│              Storage Layer                  │
│          Binary File Persistence           │
└─────────────────────────────────────────────┘
```

---

## Core Data Structures & Complexity

| Structure | File | Purpose | Complexity |
|-----------|------|---------|-----------|
| **B+ Tree** (order 4) | `core/bplus_tree.hpp` | Record indexing | Insert/Search O(log n), Range O(log n + k) |
| **LRU Cache** | `core/lru_cache.hpp` | Record caching | Get/Put O(1) |
| **Min-Heap** | `core/min_heap.hpp` | Invoice scheduler | Push/Pop O(log n) |
| **Snowflake ID** | `core/snowflake.hpp` | Unique IDs | Generate O(1) |
| **Sliding Window** | `service/fraud_detector.hpp` | Fraud analysis | Check O(1) amortized |
| **Hash Map** (unordered) | Throughout | O(1) lookups | O(1) average |
| **Directed Graph** | `service/graph_billing.hpp` | Billing chains | BFS O(V+E), Dijkstra O((V+E) log V) |
| **Slab Allocator** | `core/memory_pool.hpp` | Object pooling | Alloc/Free O(1) |

---

## Feature Modules

### 1. Customer Management
- CRUD with B+ Tree indexing + LRU cache
- Dynamic **credit scoring** (400–820 scale)
- 4-tier system: Bronze → Silver → Gold → Enterprise
- Suspend/Activate lifecycle

### 2. Invoice & Billing Engine
- **Factory Pattern** for 3 invoice types:
  - One-Time, Recurring (daily/weekly/monthly/yearly), Prorated
- **Observer Pattern** for billing events → Notifications
- Multi-threaded batch generation (`std::thread`)
- **Min-Heap scheduler** for due-date ordering

### 3. Payment Processing
- **Strategy Pattern** — 3 payment gateways (Credit Card, Bank Transfer, Wallet)
- Exponential backoff retry (5 retries, 200ms base)
- Partial/overpayment handling with credit balance
- Refund processing

### 4. Fraud Detection
- **Sliding window** frequency analysis (60s default window)
- 4 fraud rules: frequency, large amount, multiple large tx, window total
- Risk score accumulation (0.0–1.0)
- Per-customer isolation

### 5. Reports & Analytics
- **Aging Report** — bucket sort: 0-30, 31-60, 61-90, 90+ days
- **Revenue Forecasting** — Simple Moving Average (configurable window)
- **Customer Lifetime Value (CLV)** — avg_monthly × 24 months
- CSV/JSON export

### 6. Graph Billing Chains
- BFS **topological sort** for dependency ordering
- **Dijkstra** minimum-cost billing path
- Cycle detection (prevents infinite billing loops)

### 7. Notification System
- **Priority Queue** dispatch (CRITICAL → HIGH → MEDIUM → LOW)
- **State Machine** escalation: ACTIVE → WARNED → ESCALATED → SUSPENDED → CLOSED
- 3 channels: EMAIL, SMS, IN-APP

### 8. Security & Audit
- **RBAC** — 13 bitmask permissions, 4 predefined roles
- **Append-only audit trail** with XOR checksum verification
- XOR cipher + Caesar cipher + AES-128 stub
- Password hashing

---

## Default User Accounts

| Username | Password | Role | Permissions |
|----------|----------|------|-------------|
| `admin` | `admin123` | ADMIN | All |
| `manager` | `manager123` | MANAGER | Read + Write + Audit + Export |
| `agent1` | `agent123` | BILLING | Read + Invoice + Payment |
| `viewer` | `readonly` | READ_ONLY | Read + Reports |

---

## Design Patterns Used

| Pattern | Applied In |
|---------|-----------|
| **Factory** | `BillingEngine` — invoice type creation |
| **Strategy** | `PaymentProcessor` — gateway selection; `DiscountEngine` — rules |
| **Observer** | `BillingEngine` → `NotificationService` |
| **Singleton** | `AuditService`, `SnowflakeGenerator` |
| **State Machine** | Notification escalation |

---

## Tax Jurisdictions (15+)

US-CA, US-NY, US-TX, US-WA, US-FL, US-IL, EU, IN, UK, SG, AU, JP, CA, AE (UAE), HK (zero-tax)

---

## Project Structure

```
Billing System/
├── src/
│   ├── core/           # Data structures (B+Tree, LRU, MinHeap, Snowflake, MemoryPool)
│   ├── models/         # Domain models (Customer, Invoice, Payment, Notification, AuditLog)
│   ├── repository/     # File-backed persistence
│   ├── service/        # Business logic (11 service modules)
│   ├── cli/            # CLI modules (5 modules)
│   ├── data/           # Sample data loader
│   └── main.cpp        # Application entry point
├── tests/              # Unit test harness + 8 test suites (61 tests)
├── data/               # Runtime binary data files
├── exports/            # CSV/JSON report exports
├── CMakeLists.txt      # CMake build (requires cmake 3.16+)
├── Makefile            # Simple make build
└── README.md
```

---

## Building with CMake

```bash
mkdir -p build/cmake && cd build/cmake
cmake ../.. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
ctest
```

---

## Unit Test Results

```
=== B+ Tree          ===  8/8  passed
=== LRU Cache        ===  8/8  passed
=== Snowflake ID     ===  6/6  passed
=== Billing Engine   ===  8/8  passed
=== Payment Processor===  9/9  passed
=== Fraud Detector   ===  6/6  passed
=== Report Service   ===  7/7  passed
=== RBAC             ===  9/9  passed
TOTAL: 61 passed, 0 failed
```
