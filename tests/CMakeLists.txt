cmake_minimum_required(VERSION 3.20)

# Find GTest
find_package(GTest REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# For now, just build minimal test to verify CI setup
add_executable(run_tests test_minimal.cpp)

# Link libraries - use conditional Conan vs system targets
if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    # Conan 1.x target names
    target_link_libraries(run_tests
        PRIVATE
            CONAN_PKG::gtest
            ${PROJECT_NAME}_lib
    )
else()
    # System package target names
    target_link_libraries(run_tests
        PRIVATE
            GTest::gtest
            GTest::gtest_main
            ${PROJECT_NAME}_lib
    )
endif()

# Set test properties
set_target_properties(run_tests PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Add tests to CTest
include(GoogleTest)
gtest_discover_tests(run_tests)
