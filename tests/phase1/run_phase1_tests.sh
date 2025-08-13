#!/bin/bash

# Phase 1 Test Runner: Foundation & Infrastructure
# Tests T1.1.1 through T1.3.5

set -euo pipefail

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Default values
BUILD_DIR="../../build"
VERBOSE=false
PARALLEL_JOBS=$(nproc 2>/dev/null || echo 4)

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --jobs)
            PARALLEL_JOBS="$2"
            shift 2
            ;;
        --help)
            echo "Phase 1 Test Runner - Foundation & Infrastructure"
            echo ""
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --build-dir DIR    Build directory [default: ../../build]"
            echo "  --verbose         Enable verbose output"
            echo "  --jobs NUM        Number of parallel jobs [default: nproc]"
            echo "  --help            Show this help message"
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

log_info "Starting Phase 1 Tests: Foundation & Infrastructure"
log_info "Build directory: $BUILD_DIR"
log_info "Parallel jobs: $PARALLEL_JOBS"

# Function to run a test category
run_test_category() {
    local category=$1
    local description=$2
    
    log_info "Running $description..."
    
    if [[ "$VERBOSE" == true ]]; then
        gtest_filter="*${category}*" "$BUILD_DIR/tests/phase1/phase1_tests" --gtest_color=yes
    else
        gtest_filter="*${category}*" "$BUILD_DIR/tests/phase1/phase1_tests" --gtest_color=yes --gtest_brief=1
    fi
    
    if [[ $? -eq 0 ]]; then
        log_success "$description completed successfully"
        return 0
    else
        log_error "$description failed"
        return 1
    fi
}

# Check if build directory exists
if [[ ! -d "$BUILD_DIR" ]]; then
    log_error "Build directory '$BUILD_DIR' does not exist"
    log_info "Please run the build script first: ../../scripts/setup.sh"
    exit 1
fi

# Check if Phase 1 test executable exists
if [[ ! -f "$BUILD_DIR/tests/phase1/phase1_tests" ]]; then
    log_error "Phase 1 test executable not found"
    log_info "Building Phase 1 tests..."
    
    cd "$BUILD_DIR"
    if ! make phase1_tests -j"$PARALLEL_JOBS"; then
        log_error "Failed to build Phase 1 tests"
        exit 1
    fi
    cd - > /dev/null
fi

# Test execution tracking
TESTS_PASSED=0
TESTS_FAILED=0
FAILED_TESTS=()

# Test T1.1: Build System Tests
log_info "=== Testing Category T1.1: Build System ==="
if run_test_category "BuildSystem" "Build System Tests"; then
    ((TESTS_PASSED++))
else
    ((TESTS_FAILED++))
    FAILED_TESTS+=("Build System Tests")
fi

# Test T1.2: CI/CD Pipeline Tests  
log_info "=== Testing Category T1.2: CI/CD Pipeline ==="
if run_test_category "CIPipeline" "CI/CD Pipeline Tests"; then
    ((TESTS_PASSED++))
else
    ((TESTS_FAILED++))
    FAILED_TESTS+=("CI/CD Pipeline Tests")
fi

# Test T1.3: Testing Framework Tests
log_info "=== Testing Category T1.3: Testing Framework ==="
if run_test_category "TestingFramework" "Testing Framework Tests"; then
    ((TESTS_PASSED++))
else
    ((TESTS_FAILED++))
    FAILED_TESTS+=("Testing Framework Tests")
fi

# Additional validation tests
log_info "=== Running Additional Validation Tests ==="

# Check project structure
log_info "Validating project structure..."
required_dirs=("src" "include" "tests" "scripts" "configs")
for dir in "${required_dirs[@]}"; do
    if [[ ! -d "../../$dir" ]]; then
        log_error "Required directory missing: $dir"
        ((TESTS_FAILED++))
        FAILED_TESTS+=("Project Structure - $dir")
    else
        log_success "Directory found: $dir"
    fi
done

# Check essential files
log_info "Validating essential files..."
required_files=(
    "../../CMakeLists.txt"
    "../../conanfile.txt" 
    "../../README.md"
    "../../LICENSE"
    "../../.gitignore"
    "../../scripts/setup.sh"
)

for file in "${required_files[@]}"; do
    if [[ ! -f "$file" ]]; then
        log_error "Required file missing: $file"
        ((TESTS_FAILED++))
        FAILED_TESTS+=("Essential Files - $(basename $file)")
    else
        log_success "File found: $(basename $file)"
    fi
done

# Test summary
log_info "=== Phase 1 Test Summary ==="
log_info "Tests passed: $TESTS_PASSED"
log_info "Tests failed: $TESTS_FAILED"

if [[ $TESTS_FAILED -eq 0 ]]; then
    log_success "All Phase 1 tests passed! Foundation & Infrastructure is ready."
    echo ""
    log_info "Phase 1 Completion Status:"
    log_success "✅ T1.1: Build System Tests"
    log_success "✅ T1.2: CI/CD Pipeline Tests" 
    log_success "✅ T1.3: Testing Framework Tests"
    echo ""
    log_info "Next steps:"
    log_info "  1. Run Phase 2 tests: ../phase2/run_phase2_tests.sh"
    log_info "  2. Execute full build: ../../scripts/setup.sh"
    log_info "  3. Run integration tests: ../integration/run_integration_tests.sh"
    exit 0
else
    log_error "Phase 1 tests failed. Issues found:"
    for failed_test in "${FAILED_TESTS[@]}"; do
        log_error "  - $failed_test"
    done
    echo ""
    log_info "Please fix the issues above before proceeding to Phase 2."
    exit 1
fi
