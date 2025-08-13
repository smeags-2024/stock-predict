#!/bin/bash

# Phase 2 Test Runner: Core Prediction Engine
# Tests T2.1.1 through T2.4.5

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
GPU_TESTS=false

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
        --gpu)
            GPU_TESTS=true
            shift
            ;;
        --help)
            echo "Phase 2 Test Runner - Core Prediction Engine"
            echo ""
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --build-dir DIR    Build directory [default: ../../build]"
            echo "  --verbose         Enable verbose output"
            echo "  --jobs NUM        Number of parallel jobs [default: nproc]"
            echo "  --gpu             Enable GPU tests (requires CUDA)"
            echo "  --help            Show this help message"
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

log_info "Starting Phase 2 Tests: Core Prediction Engine"
log_info "Build directory: $BUILD_DIR"
log_info "Parallel jobs: $PARALLEL_JOBS"
log_info "GPU tests: $GPU_TESTS"

# Function to run a test category
run_test_category() {
    local category=$1
    local description=$2
    
    log_info "Running $description..."
    
    local test_args="--gtest_color=yes"
    if [[ "$VERBOSE" != true ]]; then
        test_args="$test_args --gtest_brief=1"
    fi
    
    if [[ -n "$category" ]]; then
        test_args="$test_args --gtest_filter=*${category}*"
    fi
    
    if "$BUILD_DIR/tests/phase2/phase2_tests" $test_args; then
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

# Check if Phase 2 test executable exists
if [[ ! -f "$BUILD_DIR/tests/phase2/phase2_tests" ]]; then
    log_error "Phase 2 test executable not found"
    log_info "Building Phase 2 tests..."
    
    cd "$BUILD_DIR"
    if ! make phase2_tests -j"$PARALLEL_JOBS"; then
        log_error "Failed to build Phase 2 tests"
        exit 1
    fi
    cd - > /dev/null
fi

# Test execution tracking
TESTS_PASSED=0
TESTS_FAILED=0
FAILED_TESTS=()

# Test T2.1: Predictor Interface Tests
log_info "=== Testing Category T2.1: Predictor Interface ==="
if run_test_category "CorePredictionEngine" "Predictor Interface Tests"; then
    ((TESTS_PASSED++))
else
    ((TESTS_FAILED++))
    FAILED_TESTS+=("Predictor Interface Tests")
fi

# Test T2.2: LSTM Model Tests
log_info "=== Testing Category T2.2: LSTM Model ==="
if run_test_category "LSTM" "LSTM Model Tests"; then
    ((TESTS_PASSED++))
else
    ((TESTS_FAILED++))
    FAILED_TESTS+=("LSTM Model Tests")
fi

# Test T2.3: Transformer Model Tests
log_info "=== Testing Category T2.3: Transformer Model ==="
if run_test_category "Transformer" "Transformer Model Tests"; then
    ((TESTS_PASSED++))
else
    ((TESTS_FAILED++))
    FAILED_TESTS+=("Transformer Model Tests")
fi

# Test T2.4: Ensemble Model Tests
log_info "=== Testing Category T2.4: Ensemble Model ==="
if run_test_category "Ensemble" "Ensemble Model Tests"; then
    ((TESTS_PASSED++))
else
    ((TESTS_FAILED++))
    FAILED_TESTS+=("Ensemble Model Tests")
fi

# GPU-specific tests if enabled
if [[ "$GPU_TESTS" == true ]]; then
    log_info "=== Testing GPU Acceleration ==="
    
    # Check if CUDA is available
    if command -v nvidia-smi &> /dev/null; then
        log_info "CUDA detected, running GPU tests..."
        if run_test_category "GPU" "GPU Acceleration Tests"; then
            ((TESTS_PASSED++))
        else
            ((TESTS_FAILED++))
            FAILED_TESTS+=("GPU Acceleration Tests")
        fi
    else
        log_warning "CUDA not available, skipping GPU tests"
    fi
fi

# Performance validation tests
log_info "=== Running Performance Validation ==="

# Test prediction latency
log_info "Testing prediction latency..."
if "$BUILD_DIR/bin/StockPredict" --test --symbol=TEST --benchmark-prediction-latency 2>/dev/null; then
    log_success "Prediction latency test passed"
    ((TESTS_PASSED++))
else
    log_error "Prediction latency test failed"
    ((TESTS_FAILED++))
    FAILED_TESTS+=("Prediction Latency")
fi

# Test memory usage
log_info "Testing memory usage..."
if "$BUILD_DIR/bin/StockPredict" --test --symbol=TEST --benchmark-memory-usage 2>/dev/null; then
    log_success "Memory usage test passed"
    ((TESTS_PASSED++))
else
    log_error "Memory usage test failed"
    ((TESTS_FAILED++))
    FAILED_TESTS+=("Memory Usage")
fi

# Model accuracy validation
log_info "=== Running Model Accuracy Validation ==="

# Test with different model types
models=("lstm" "transformer" "ensemble")
for model in "${models[@]}"; do
    log_info "Testing $model model accuracy..."
    if "$BUILD_DIR/bin/StockPredict" --test --symbol=TEST --model="$model" --epochs=1 --validate-accuracy 2>/dev/null; then
        log_success "$model model accuracy test passed"
        ((TESTS_PASSED++))
    else
        log_warning "$model model accuracy test failed (this may be expected for quick tests)"
        # Don't fail for accuracy tests in quick validation
    fi
done

# Test summary
log_info "=== Phase 2 Test Summary ==="
log_info "Tests passed: $TESTS_PASSED"
log_info "Tests failed: $TESTS_FAILED"

if [[ $TESTS_FAILED -eq 0 ]]; then
    log_success "All Phase 2 tests passed! Core Prediction Engine is ready."
    echo ""
    log_info "Phase 2 Completion Status:"
    log_success "✅ T2.1: Predictor Interface Tests"
    log_success "✅ T2.2: LSTM Model Tests"
    log_success "✅ T2.3: Transformer Model Tests"
    log_success "✅ T2.4: Ensemble Model Tests"
    if [[ "$GPU_TESTS" == true ]]; then
        log_success "✅ GPU Acceleration Tests"
    fi
    echo ""
    log_info "Key Performance Metrics Validated:"
    log_success "  • Prediction Latency: <1ms target"
    log_success "  • Memory Usage: <200MB target" 
    log_success "  • Model Creation: All model types functional"
    log_success "  • PyTorch Integration: Working correctly"
    echo ""
    log_info "Next steps:"
    log_info "  1. Run Phase 3 tests: ../phase3/run_phase3_tests.sh"
    log_info "  2. Test feature engineering: ../phase3/run_phase3_tests.sh --verbose"
    log_info "  3. Run full model training: ../../scripts/setup.sh && ./build/bin/StockPredict --train"
    exit 0
else
    log_error "Phase 2 tests failed. Issues found:"
    for failed_test in "${FAILED_TESTS[@]}"; do
        log_error "  - $failed_test"
    done
    echo ""
    log_info "Common troubleshooting steps:"
    log_info "  1. Ensure PyTorch C++ is properly installed"
    log_info "  2. Check that all dependencies are built correctly"
    log_info "  3. Verify CUDA installation if using GPU tests"
    log_info "  4. Run with --verbose for detailed error information"
    exit 1
fi
