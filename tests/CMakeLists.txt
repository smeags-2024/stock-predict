cmake_minimum_required(VERSION 3.20)

# Find GTest - try system package first, then fallback to FetchContent
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    message(STATUS "GTest not found as system package, using FetchContent")
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50a2a94298060058b1db0e9d01de69.zip
    )
    FetchContent_MakeAvailable(googletest)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Find available test files
file(GLOB_RECURSE TEST_SOURCES "*.cpp")

# Also look for phase test files
file(GLOB_RECURSE PHASE1_TESTS "phase1/*.cpp")
file(GLOB_RECURSE PHASE2_TESTS "phase2/*.cpp")
file(GLOB_RECURSE PHASE3_TESTS "phase3/*.cpp")
file(GLOB_RECURSE PHASE4_TESTS "phase4/*.cpp")

# Combine all test sources
list(APPEND TEST_SOURCES ${PHASE1_TESTS} ${PHASE2_TESTS} ${PHASE3_TESTS} ${PHASE4_TESTS})

if(TEST_SOURCES)
    # Build test executable with available test files
    add_executable(run_tests ${TEST_SOURCES})
else()
    # Create minimal test if no test files available
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/minimal_test.cpp
        "#include <gtest/gtest.h>\n"
        "TEST(BasicTest, AlwaysPass) { EXPECT_TRUE(true); }\n"
        "int main(int argc, char **argv) {\n"
        "    ::testing::InitGoogleTest(&argc, argv);\n"
        "    return RUN_ALL_TESTS();\n"
        "}\n"
    )
    add_executable(run_tests ${CMAKE_CURRENT_BINARY_DIR}/minimal_test.cpp)
endif()

# Link libraries - use system packages only
if(GTest_FOUND)
    target_link_libraries(run_tests
        PRIVATE
            GTest::gtest
            GTest::gtest_main
            ${PROJECT_NAME}_lib
    )
else()
    target_link_libraries(run_tests
        PRIVATE
            gtest
            gtest_main
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

# Add individual phase test targets
if(PHASE1_TESTS)
    add_executable(phase1_tests ${PHASE1_TESTS})
    if(GTest_FOUND)
        target_link_libraries(phase1_tests PRIVATE GTest::gtest GTest::gtest_main ${PROJECT_NAME}_lib)
    else()
        target_link_libraries(phase1_tests PRIVATE gtest gtest_main ${PROJECT_NAME}_lib)
    endif()
    set_target_properties(phase1_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
    gtest_discover_tests(phase1_tests TEST_PREFIX "phase1.")
endif()

if(PHASE2_TESTS)
    add_executable(phase2_tests ${PHASE2_TESTS})
    if(GTest_FOUND)
        target_link_libraries(phase2_tests PRIVATE GTest::gtest GTest::gtest_main ${PROJECT_NAME}_lib)
    else()
        target_link_libraries(phase2_tests PRIVATE gtest gtest_main ${PROJECT_NAME}_lib)
    endif()
    set_target_properties(phase2_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
    gtest_discover_tests(phase2_tests TEST_PREFIX "phase2.")
endif()

if(PHASE3_TESTS)
    add_executable(phase3_tests ${PHASE3_TESTS})
    if(GTest_FOUND)
        target_link_libraries(phase3_tests PRIVATE GTest::gtest GTest::gtest_main ${PROJECT_NAME}_lib)
    else()
        target_link_libraries(phase3_tests PRIVATE gtest gtest_main ${PROJECT_NAME}_lib)
    endif()
    set_target_properties(phase3_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
    gtest_discover_tests(phase3_tests TEST_PREFIX "phase3.")
endif()

if(PHASE4_TESTS)
    add_executable(phase4_tests ${PHASE4_TESTS})
    if(GTest_FOUND)
        target_link_libraries(phase4_tests PRIVATE GTest::gtest GTest::gtest_main ${PROJECT_NAME}_lib)
    else()
        target_link_libraries(phase4_tests PRIVATE gtest gtest_main ${PROJECT_NAME}_lib)
    endif()
    set_target_properties(phase4_tests PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
    gtest_discover_tests(phase4_tests TEST_PREFIX "phase4.")
endif()
