cmake_minimum_required(VERSION 3.20)

# Find Google Benchmark
find_package(benchmark REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Benchmark source files
file(GLOB_RECURSE BENCHMARK_SOURCES "*.cpp")

# Create benchmark executable
add_executable(run_benchmarks ${BENCHMARK_SOURCES})

# Link libraries
target_link_libraries(run_benchmarks
    PRIVATE
        ${PROJECT_NAME}_lib
        benchmark::benchmark
        benchmark::benchmark_main
)

# Add Eigen if available
if(TARGET Eigen3::Eigen)
    target_link_libraries(run_benchmarks PRIVATE Eigen3::Eigen)
endif()

# Set benchmark properties
set_target_properties(run_benchmarks PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
