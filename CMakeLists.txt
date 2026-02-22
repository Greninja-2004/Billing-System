cmake_minimum_required(VERSION 3.16)
project(BillingSystem VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

# Directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# ============================================================================
# Main executable: billing_system
# ============================================================================
set(MAIN_SOURCES
    src/main.cpp
)

add_executable(billing_system ${MAIN_SOURCES})
target_compile_options(billing_system PRIVATE -Wall -Wextra)
target_link_libraries(billing_system pthread)

# Ensure data/ and exports/ exist at build tree
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/data)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/exports)

# Copy run from build/: the executable searches relative paths for data/
add_custom_command(TARGET billing_system POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory data
    COMMAND ${CMAKE_COMMAND} -E make_directory exports
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# ============================================================================
# Test executable: billing_tests
# ============================================================================
set(TEST_SOURCES
    tests/test_runner.cpp
    tests/test_bplus_tree.cpp
    tests/test_lru_cache.cpp
    tests/test_snowflake.cpp
    tests/test_billing_engine.cpp
    tests/test_payment_processor.cpp
    tests/test_fraud_detector.cpp
    tests/test_report_service.cpp
    tests/test_rbac.cpp
)

add_executable(billing_tests ${TEST_SOURCES})
target_include_directories(billing_tests PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/tests)
target_link_libraries(billing_tests pthread)
target_compile_options(billing_tests PRIVATE -Wall -Wextra)

# Ensure data dir for tests
add_custom_command(TARGET billing_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory data
    COMMAND ${CMAKE_COMMAND} -E make_directory exports
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# ============================================================================
# CTest integration
# ============================================================================
enable_testing()
add_test(NAME UnitTests COMMAND billing_tests)

# ============================================================================
# Install targets
# ============================================================================
install(TARGETS billing_system RUNTIME DESTINATION bin)
