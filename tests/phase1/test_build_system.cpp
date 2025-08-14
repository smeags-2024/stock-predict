#include <gtest/gtest.h>
#include <cstdlib>
#include <filesystem>
#include <fstream>

// Phase 1: Foundation & Infrastructure Tests
// Test T1.1.1-T1.1.5: Build System Tests

namespace phase1 {
namespace build_system {

class BuildSystemTest : public ::testing::Test {
   protected:
    void SetUp() override {
        project_root_ = std::filesystem::current_path().parent_path().parent_path();
        build_dir_ = project_root_ / "build";
    }

    std::filesystem::path project_root_;
    std::filesystem::path build_dir_;
};

// Test T1.1.1: CMake configuration validation
TEST_F(BuildSystemTest, CMakeConfigurationValidation) {
    auto cmake_file = project_root_ / "CMakeLists.txt";
    EXPECT_TRUE(std::filesystem::exists(cmake_file))
        << "CMakeLists.txt should exist in project root";

    std::ifstream file(cmake_file);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Check for required CMake components
    EXPECT_NE(content.find("cmake_minimum_required"), std::string::npos)
        << "CMakeLists.txt should specify minimum CMake version";
    EXPECT_NE(content.find("project("), std::string::npos)
        << "CMakeLists.txt should define project name";
    EXPECT_NE(content.find("CMAKE_CXX_STANDARD"), std::string::npos)
        << "CMakeLists.txt should set C++ standard";
    EXPECT_NE(content.find("find_package(Torch"), std::string::npos)
        << "CMakeLists.txt should find PyTorch";
    EXPECT_NE(content.find("find_package(Qt6"), std::string::npos)
        << "CMakeLists.txt should find Qt6";
}

// Test T1.1.2: System dependency resolution
TEST_F(BuildSystemTest, SystemDependencyResolution) {
    auto cmake_file = project_root_ / "CMakeLists.txt";
    EXPECT_TRUE(std::filesystem::exists(cmake_file))
        << "CMakeLists.txt should exist in project root";

    std::ifstream file(cmake_file);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Check for required system package finds
    EXPECT_NE(content.find("find_package(Eigen3"), std::string::npos)
        << "CMakeLists.txt should find Eigen3 system package";
    EXPECT_NE(content.find("find_package(nlohmann_json"), std::string::npos)
        << "CMakeLists.txt should find nlohmann_json system package";
    EXPECT_NE(content.find("find_package(spdlog"), std::string::npos)
        << "CMakeLists.txt should find spdlog system package";
}

// Test T1.1.3: Multi-platform compilation
TEST_F(BuildSystemTest, MultiPlatformCompilation) {
    // This test verifies that the build system supports multiple platforms
    auto cmake_file = project_root_ / "CMakeLists.txt";
    std::ifstream file(cmake_file);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Check for platform-specific considerations
    EXPECT_NE(content.find("CMAKE_BUILD_TYPE"), std::string::npos)
        << "CMakeLists.txt should handle build types";

    // Verify compiler flags are set
    EXPECT_NE(content.find("CMAKE_CXX_FLAGS"), std::string::npos)
        << "CMakeLists.txt should set compiler flags";
}

// Test T1.1.4: Build script functionality
TEST_F(BuildSystemTest, BuildScriptFunctionality) {
    auto setup_script = project_root_ / "scripts" / "setup.sh";
    EXPECT_TRUE(std::filesystem::exists(setup_script)) << "setup.sh script should exist";

    // Check if script is executable
    auto perms = std::filesystem::status(setup_script).permissions();
    EXPECT_NE(perms & std::filesystem::perms::owner_exec, std::filesystem::perms::none)
        << "setup.sh should be executable";

    // Verify script contains essential commands
    std::ifstream file(setup_script);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    EXPECT_NE(content.find("cmake"), std::string::npos) << "setup.sh should run cmake";
    EXPECT_NE(content.find("make"), std::string::npos) << "setup.sh should use make for building";
}

// Test T1.1.5: Optimization flag verification
TEST_F(BuildSystemTest, OptimizationFlagVerification) {
    auto cmake_file = project_root_ / "CMakeLists.txt";
    std::ifstream file(cmake_file);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Check for optimization flags
    EXPECT_NE(content.find("CMAKE_CXX_FLAGS_RELEASE"), std::string::npos)
        << "CMakeLists.txt should set release flags";
    EXPECT_NE(content.find("CMAKE_CXX_FLAGS_DEBUG"), std::string::npos)
        << "CMakeLists.txt should set debug flags";
    EXPECT_NE(content.find("-O3"), std::string::npos)
        << "CMakeLists.txt should include O3 optimization";
    EXPECT_NE(content.find("-march=native"), std::string::npos)
        << "CMakeLists.txt should include native architecture optimization";
}

}  // namespace build_system
}  // namespace phase1
