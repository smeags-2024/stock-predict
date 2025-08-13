cmake_minimum_required(VERSION 3.20)

# Find GTest
find_package(GTest REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Test source files
file(GLOB_RECURSE TEST_SOURCES "*.cpp")

# Create test executable
add_executable(run_tests ${TEST_SOURCES})

# Link libraries
target_link_libraries(run_tests
    PRIVATE
        ${PROJECT_NAME}_lib
        GTest::gtest
        GTest::gtest_main
        ${TORCH_LIBRARIES}
        Eigen3::Eigen
)

# Set test properties
set_target_properties(run_tests PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Add tests to CTest
include(GoogleTest)
gtest_discover_tests(run_tests)
