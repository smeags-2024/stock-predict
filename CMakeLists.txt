cmake_minimum_required(VERSION 3.20)
project(StockPredict VERSION 1.0.0 LANGUAGES CXX)

# Set policies to avoid warnings
cmake_policy(SET CMP0146 OLD)  # Allow FindCUDA module

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(BUILD_DOCS "Build documentation" OFF)
option(BUILD_TESTS "Build tests" ON)
option(BUILD_BENCHMARKS "Build benchmarks" OFF)

# Find required packages - use system packages only
find_package(PkgConfig REQUIRED)

# Core dependencies using system packages
find_package(Eigen3 REQUIRED)
find_package(nlohmann_json REQUIRED)

# Try to find spdlog, fallback to header-only if not found
find_package(spdlog QUIET)
if(NOT spdlog_FOUND)
    message(STATUS "spdlog not found as package, will use header-only mode")
    # We'll include it directly in the project
endif()

# Try to find fmt, which is required by spdlog
find_package(fmt QUIET)
if(NOT fmt_FOUND)
    message(STATUS "fmt not found as package, will use embedded version")
endif()

# Testing framework
if(BUILD_TESTS)
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        message(STATUS "GTest not found, will download via FetchContent")
        include(FetchContent)
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/03597a01ee50a2a94298060058b1db0e9d01de69.zip
        )
        FetchContent_MakeAvailable(googletest)
    endif()
endif()

# Optional advanced packages (complex dependencies)
find_package(Torch QUIET)
find_package(Arrow QUIET)
find_package(HDF5 QUIET)
find_package(OpenMP QUIET)
find_package(cpprest QUIET)

# Qt6 packages for GUI (optional for now)
find_package(Qt6 QUIET COMPONENTS 
    Core 
    Widgets 
    Charts 
    Network 
    Concurrent
)

if(Qt6_FOUND)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)
    add_compile_definitions(QT_CHARTS_LIB)
    message(STATUS "Qt6 found - GUI will be built")
else()
    message(STATUS "Qt6 not found - GUI will be skipped")
endif()

# Optional packages for development
find_package(CUDA QUIET)
find_package(GTest QUIET)
find_package(benchmark QUIET)

# Set feature flags based on what was found
if(Torch_FOUND)
    add_compile_definitions(HAS_PYTORCH)
    message(STATUS "PyTorch found - ML models will use PyTorch backend")
else()
    message(STATUS "PyTorch not found - ML models will use simple implementations")
endif()

if(Arrow_FOUND)
    add_compile_definitions(HAS_ARROW)
    message(STATUS "Apache Arrow found - efficient data storage enabled")
endif()

if(HDF5_FOUND)
    add_compile_definitions(HAS_HDF5)
    message(STATUS "HDF5 found - HDF5 data storage enabled")
endif()

if(OpenMP_FOUND)
    add_compile_definitions(HAS_OPENMP)
    message(STATUS "OpenMP found - parallel processing enabled")
endif()

# Project information (duplicate - should be removed but kept for now)
# project(StockPredict 
#     VERSION 1.0.0
#     DESCRIPTION "Advanced Stock Price Prediction System"
#     LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")

# Optional packages
find_package(CUDA QUIET)
find_package(GTest QUIET)
find_package(benchmark QUIET)

# Include directories
include_directories(include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# Source files
file(GLOB_RECURSE SOURCES "src/core/*.cpp" "src/features/*.cpp" "src/risk/*.cpp" "src/data/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp" "include/*.h")

# GUI source files
file(GLOB_RECURSE GUI_SOURCES "src/gui/*.cpp")
file(GLOB_RECURSE GUI_HEADERS "include/stock_predict/gui/*.h")

# Qt resource files (only if Qt6 found)
if(Qt6_FOUND)
    qt6_add_resources(GUI_RESOURCES src/gui/resources.qrc)
else()
    set(GUI_RESOURCES "")
endif()

# Remove main files from library sources
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
list(REMOVE_ITEM GUI_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/gui_main.cpp")

# Create library (minimal version - only if sources exist)
if(SOURCES)
    add_library(${PROJECT_NAME}_lib ${SOURCES} ${HEADERS})
    
    # Target properties
    set_target_properties(${PROJECT_NAME}_lib PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        POSITION_INDEPENDENT_CODE ON
    )
else()
    # Create header-only interface library if no sources
    add_library(${PROJECT_NAME}_lib INTERFACE)
    target_include_directories(${PROJECT_NAME}_lib INTERFACE include)
endif()

# Link libraries - use system packages only
target_link_libraries(${PROJECT_NAME}_lib
    PUBLIC
        Eigen3::Eigen
        nlohmann_json::nlohmann_json
)

# Link spdlog if found, otherwise we'll use header-only mode
if(spdlog_FOUND)
    target_link_libraries(${PROJECT_NAME}_lib PUBLIC spdlog::spdlog)
else()
    # Include spdlog headers directly from system or bundled
    target_include_directories(${PROJECT_NAME}_lib PUBLIC 
        /usr/include/spdlog 
        /usr/local/include/spdlog
    )
    # Link fmt if available
    if(fmt_FOUND)
        target_link_libraries(${PROJECT_NAME}_lib PUBLIC fmt::fmt)
    endif()
endif()

# Link optional advanced libraries
if(Torch_FOUND)
    target_link_libraries(${PROJECT_NAME}_lib PUBLIC ${TORCH_LIBRARIES})
endif()

if(Arrow_FOUND)
    target_link_libraries(${PROJECT_NAME}_lib PUBLIC Arrow::arrow Arrow::arrow_dataset)
endif()

if(HDF5_FOUND)
    target_link_libraries(${PROJECT_NAME}_lib PUBLIC ${HDF5_LIBRARIES})
endif()

if(OpenMP_FOUND)
    target_link_libraries(${PROJECT_NAME}_lib PUBLIC OpenMP::OpenMP_CXX)
endif()

if(cpprest_FOUND)
    target_link_libraries(${PROJECT_NAME}_lib PUBLIC cpprest::cpprest)
endif()

# Create GUI library (if Qt6 is available)
if(Qt6_FOUND)
    add_library(${PROJECT_NAME}_gui ${GUI_SOURCES} ${GUI_HEADERS} ${GUI_RESOURCES})

    set_target_properties(${PROJECT_NAME}_gui PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        POSITION_INDEPENDENT_CODE ON
    )

    target_link_libraries(${PROJECT_NAME}_gui
        PUBLIC
            ${PROJECT_NAME}_lib
            Qt6::Core
            Qt6::Widgets
            Qt6::Charts
            Qt6::Network
            Qt6::Concurrent
    )
else()
    # Create dummy GUI library for compatibility
    add_library(${PROJECT_NAME}_gui INTERFACE)
    message(STATUS "Creating dummy GUI library - Qt6 not available")
endif()

# CUDA support
if(CUDA_FOUND)
    enable_language(CUDA)
    target_compile_definitions(${PROJECT_NAME}_lib PUBLIC CUDA_ENABLED)
    target_link_libraries(${PROJECT_NAME}_lib PUBLIC ${CUDA_LIBRARIES})
endif()

# Main executable (console)
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_lib)

# GUI executable (only if Qt6 is available)
if(Qt6_FOUND)
    add_executable(${PROJECT_NAME}_gui_app src/gui/gui_main.cpp)
    target_link_libraries(${PROJECT_NAME}_gui_app ${PROJECT_NAME}_gui ${PROJECT_NAME}_lib)
    # Set executable names
    set_target_properties(${PROJECT_NAME}_gui_app PROPERTIES OUTPUT_NAME "${PROJECT_NAME}_gui")
endif()

# Install targets
install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}_lib
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

if(Qt6_FOUND)
    install(TARGETS ${PROJECT_NAME}_gui_app ${PROJECT_NAME}_gui
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
endif()

install(DIRECTORY include/ DESTINATION include)

# Testing
if(BUILD_TESTS)
    enable_testing()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
        add_subdirectory(tests)
    else()
        message(STATUS "Tests directory not found, skipping tests")
    endif()
endif()

# Benchmarks
if(benchmark_FOUND)
    add_subdirectory(benchmarks)
endif()

# Package configuration
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/StockPredictConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/StockPredictConfig.cmake"
    INSTALL_DESTINATION lib/cmake/StockPredict
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/StockPredictConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/StockPredictConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/StockPredictConfigVersion.cmake"
    DESTINATION lib/cmake/StockPredict
)

# Documentation build (uses docs/CMakeLists.txt)
if(BUILD_DOCS)
    add_subdirectory(docs ${CMAKE_BINARY_DIR}/documentation)
endif()
