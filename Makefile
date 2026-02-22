CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -pthread
SRC_DIR  = src
TEST_DIR = tests
BUILD    = build/make

MAIN_SRC = $(SRC_DIR)/main.cpp
TEST_SRCS = $(TEST_DIR)/test_runner.cpp \
            $(TEST_DIR)/test_bplus_tree.cpp \
            $(TEST_DIR)/test_lru_cache.cpp \
            $(TEST_DIR)/test_snowflake.cpp \
            $(TEST_DIR)/test_billing_engine.cpp \
            $(TEST_DIR)/test_payment_processor.cpp \
            $(TEST_DIR)/test_fraud_detector.cpp \
            $(TEST_DIR)/test_report_service.cpp \
            $(TEST_DIR)/test_rbac.cpp

.PHONY: all main tests clean setup

all: setup main tests

setup:
	@mkdir -p $(BUILD) data exports

main: setup
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) $(MAIN_SRC) -o $(BUILD)/billing_system
	@echo "✓ billing_system built at $(BUILD)/billing_system"

tests: setup
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -I$(TEST_DIR) $(TEST_SRCS) -o $(BUILD)/billing_tests
	@echo "✓ billing_tests built at $(BUILD)/billing_tests"

run: main
	@cd $(BUILD) && mkdir -p data exports && ./billing_system

run_tests: tests
	@cd $(BUILD) && mkdir -p data exports && ./billing_tests

demo: main
	@cd $(BUILD) && mkdir -p data exports && ./billing_system --demo

clean:
	rm -rf $(BUILD) data exports
	@echo "✓ Cleaned build artifacts"
