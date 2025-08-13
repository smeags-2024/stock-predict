cmake_minimum_required(VERSION 3.20)

# Find GTest
find_package(GTest REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# For now, just build minimal test to verify CI setup
add_executable(run_tests test_minimal.cpp)

# Link libraries - minimal dependencies only
target_link_libraries(run_tests
    PRIVATE
        GTest::gtest
        GTest::gtest_main
    )
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
