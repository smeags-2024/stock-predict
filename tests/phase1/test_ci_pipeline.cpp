#include <gtest/gtest.h>
#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <fstream>

// Phase 1: CI/CD Pipeline Tests
// Test T1.2.1-T1.2.5: CI/CD Pipeline Tests

namespace phase1 {
namespace ci_pipeline {

class CIPipelineTest : public ::testing::Test {
   protected:
    void SetUp() override {
        project_root_ = std::filesystem::current_path().parent_path().parent_path();
        workflows_dir_ = project_root_ / ".github" / "workflows";
    }

    std::filesystem::path project_root_;
    std::filesystem::path workflows_dir_;
};

// Test T1.2.1: GitHub Actions workflow validation
TEST_F(CIPipelineTest, GitHubActionsWorkflowValidation) {
    auto ci_file = workflows_dir_ / "ci.yml";
    EXPECT_TRUE(std::filesystem::exists(ci_file)) << "CI workflow file should exist";

    std::ifstream file(ci_file);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Check for required workflow components
    EXPECT_NE(content.find("name:"), std::string::npos) << "Workflow should have a name";
    EXPECT_NE(content.find("on:"), std::string::npos) << "Workflow should define triggers";
    EXPECT_NE(content.find("jobs:"), std::string::npos) << "Workflow should define jobs";
    EXPECT_NE(content.find("strategy:"), std::string::npos)
        << "Workflow should use strategy matrix";
}

// Test T1.2.2: Multi-platform testing matrix
TEST_F(CIPipelineTest, MultiPlatformTestingMatrix) {
    auto ci_file = workflows_dir_ / "ci.yml";
    std::ifstream file(ci_file);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Check for multiple platforms
    EXPECT_NE(content.find("ubuntu-latest"), std::string::npos) << "CI should test on Ubuntu";
    EXPECT_NE(content.find("macos-latest"), std::string::npos) << "CI should test on macOS";
    EXPECT_NE(content.find("windows-latest"), std::string::npos) << "CI should test on Windows";

    // Check for multiple build types
    EXPECT_NE(content.find("Debug"), std::string::npos) << "CI should test Debug builds";
    EXPECT_NE(content.find("Release"), std::string::npos) << "CI should test Release builds";
}

// Test T1.2.3: Code quality checks execution
TEST_F(CIPipelineTest, CodeQualityChecksExecution) {
    auto ci_file = workflows_dir_ / "ci.yml";
    std::ifstream file(ci_file);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Check for code quality tools
    EXPECT_NE(content.find("clang-format"), std::string::npos) << "CI should run clang-format";
    EXPECT_NE(content.find("cppcheck"), std::string::npos) << "CI should run cppcheck";
    EXPECT_NE(content.find("code-quality"), std::string::npos) << "CI should have code quality job";
}

// Test T1.2.4: Sanitizer integration testing
TEST_F(CIPipelineTest, SanitizerIntegrationTesting) {
    auto ci_file = workflows_dir_ / "ci.yml";
    std::ifstream file(ci_file);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Check for sanitizer testing
    EXPECT_NE(content.find("sanitizer"), std::string::npos) << "CI should include sanitizer tests";
    EXPECT_NE(content.find("address"), std::string::npos) << "CI should test with AddressSanitizer";
    EXPECT_NE(content.find("thread"), std::string::npos) << "CI should test with ThreadSanitizer";
}

// Test T1.2.5: Benchmark automation testing
TEST_F(CIPipelineTest, BenchmarkAutomationTesting) {
    auto ci_file = workflows_dir_ / "ci.yml";
    std::ifstream file(ci_file);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Check for benchmark automation
    EXPECT_NE(content.find("benchmark"), std::string::npos) << "CI should run benchmarks";
    EXPECT_NE(content.find("benchmark_results"), std::string::npos)
        << "CI should collect benchmark results";
}

}  // namespace ci_pipeline
}  // namespace phase1
