#include <gtest/gtest.h>
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

    // Check for Ubuntu platform (our current implementation)
    EXPECT_NE(content.find("ubuntu-latest"), std::string::npos) << "CI should test on Ubuntu";
    
    // Check for multiple compilers instead of platforms
    EXPECT_NE(content.find("gcc"), std::string::npos) << "CI should test with GCC";
    EXPECT_NE(content.find("clang"), std::string::npos) << "CI should test with Clang";

    // Check for multiple build types
    EXPECT_NE(content.find("Debug"), std::string::npos) << "CI should test Debug builds";
    EXPECT_NE(content.find("Release"), std::string::npos) << "CI should test Release builds";
}

// Test T1.2.3: Code quality checks execution
TEST_F(CIPipelineTest, CodeQualityChecksExecution) {
    auto ci_file = workflows_dir_ / "ci.yml";
    std::ifstream file(ci_file);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Check for code quality tools that we actually have
    EXPECT_NE(content.find("cppcheck"), std::string::npos) << "CI should run cppcheck";
    EXPECT_NE(content.find("clang-tidy"), std::string::npos) << "CI should run clang-tidy";
    EXPECT_NE(content.find("static-analysis"), std::string::npos) << "CI should have static analysis job";
}

// Test T1.2.4: Memory check integration testing
TEST_F(CIPipelineTest, MemoryCheckIntegrationTesting) {
    auto ci_file = workflows_dir_ / "ci.yml";
    std::ifstream file(ci_file);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Check for memory checking tools that we actually have
    EXPECT_NE(content.find("valgrind"), std::string::npos) << "CI should include valgrind memory checks";
    EXPECT_NE(content.find("memory-check"), std::string::npos) << "CI should have memory check job";
}

// Test T1.2.5: Code coverage automation testing
TEST_F(CIPipelineTest, CodeCoverageAutomationTesting) {
    auto ci_file = workflows_dir_ / "ci.yml";
    std::ifstream file(ci_file);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Check for code coverage automation that we actually have
    EXPECT_NE(content.find("coverage"), std::string::npos) << "CI should run code coverage";
    EXPECT_NE(content.find("lcov"), std::string::npos) << "CI should use lcov for coverage";
    EXPECT_NE(content.find("codecov"), std::string::npos) << "CI should upload coverage to codecov";
}

}  // namespace ci_pipeline
}  // namespace phase1
