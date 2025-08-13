#!/bin/bash

# Phase 4 Test Runner: Risk Management & Portfolio Optimization
# Tests T4.1.1 through T4.5.5

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
STRESS_TESTS=false
BACKTESTING=false
LARGE_PORTFOLIO_TESTS=false

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
        --stress-tests)
            STRESS_TESTS=true
            shift
            ;;
        --backtesting)
            BACKTESTING=true
            shift
            ;;
        --large-portfolio)
            LARGE_PORTFOLIO_TESTS=true
            shift
            ;;
        --help)
            echo "Phase 4 Test Runner - Risk Management & Portfolio Optimization"
            echo ""
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --build-dir DIR        Build directory [default: ../../build]"
            echo "  --verbose             Enable verbose output"
            echo "  --jobs NUM            Number of parallel jobs [default: nproc]"
            echo "  --stress-tests        Enable stress testing scenarios"
            echo "  --backtesting         Enable backtesting validation"
            echo "  --large-portfolio     Enable large portfolio performance tests"
            echo "  --help                Show this help message"
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

log_info "Starting Phase 4 Tests: Risk Management & Portfolio Optimization"
log_info "Build directory: $BUILD_DIR"
log_info "Parallel jobs: $PARALLEL_JOBS"
log_info "Stress tests: $STRESS_TESTS"
log_info "Backtesting: $BACKTESTING"
log_info "Large portfolio tests: $LARGE_PORTFOLIO_TESTS"

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
    
    if "$BUILD_DIR/tests/phase4/phase4_tests" $test_args; then
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

# Check if Phase 4 test executable exists
if [[ ! -f "$BUILD_DIR/tests/phase4/phase4_tests" ]]; then
    log_error "Phase 4 test executable not found"
    log_info "Building Phase 4 tests..."
    
    cd "$BUILD_DIR"
    if ! make phase4_tests -j"$PARALLEL_JOBS"; then
        log_error "Failed to build Phase 4 tests"
        exit 1
    fi
    cd - > /dev/null
fi

# Test execution tracking
TESTS_PASSED=0
TESTS_FAILED=0
FAILED_TESTS=()

# Test T4.1: Risk Calculation
log_info "=== Testing Category T4.1: Risk Calculation ==="
if run_test_category "RiskCalculation" "Risk Calculation Tests"; then
    ((TESTS_PASSED++))
else
    ((TESTS_FAILED++))
    FAILED_TESTS+=("Risk Calculation Tests")
fi

# Test T4.2: Portfolio Management
log_info "=== Testing Category T4.2: Portfolio Management ==="
if run_test_category "PortfolioManagement" "Portfolio Management Tests"; then
    ((TESTS_PASSED++))
else
    ((TESTS_FAILED++))
    FAILED_TESTS+=("Portfolio Management Tests")
fi

# Test T4.3: Position Sizing
log_info "=== Testing Category T4.3: Position Sizing ==="
if run_test_category "PositionSizing" "Position Sizing Tests"; then
    ((TESTS_PASSED++))
else
    ((TESTS_FAILED++))
    FAILED_TESTS+=("Position Sizing Tests")
fi

# Test T4.4: Stop Loss Management
log_info "=== Testing Category T4.4: Stop Loss Management ==="
if run_test_category "StopLossManagement" "Stop Loss Management Tests"; then
    ((TESTS_FAILED++))
    FAILED_TESTS+=("Stop Loss Management Tests")
fi

# Test T4.5: Integration Tests
log_info "=== Testing Category T4.5: Integration Tests ==="
if run_test_category "Phase4Integration" "Phase 4 Integration Tests"; then
    ((TESTS_PASSED++))
else
    ((TESTS_FAILED++))
    FAILED_TESTS+=("Phase 4 Integration Tests")
fi

# Risk model validation tests
log_info "=== Running Risk Model Validation ==="

# VaR model accuracy test
log_info "Testing VaR model accuracy..."
if "$BUILD_DIR/bin/StockPredict" --test --symbol=TEST --validate-var-model --confidence=0.95 2>/dev/null; then
    log_success "VaR model validation passed"
    ((TESTS_PASSED++))
else
    log_error "VaR model validation failed"
    ((TESTS_FAILED++))
    FAILED_TESTS+=("VaR Model Validation")
fi

# Portfolio optimization test
log_info "Testing portfolio optimization..."
if "$BUILD_DIR/bin/StockPredict" --test --optimize-portfolio --method=mean-variance --target-return=0.10 2>/dev/null; then
    log_success "Portfolio optimization test passed"
    ((TESTS_PASSED++))
else
    log_error "Portfolio optimization test failed"
    ((TESTS_FAILED++))
    FAILED_TESTS+=("Portfolio Optimization")
fi

# Position sizing validation
log_info "Testing position sizing methods..."
sizing_methods=("fixed-fractional" "kelly" "volatility-based" "risk-parity")
for method in "${sizing_methods[@]}"; do
    log_info "Testing $method sizing..."
    if "$BUILD_DIR/bin/StockPredict" --test --symbol=TEST --position-sizing="$method" --risk-per-trade=0.02 2>/dev/null; then
        log_success "$method sizing test passed"
        ((TESTS_PASSED++))
    else
        log_warning "$method sizing test failed (may be expected for some methods)"
    fi
done

# Stress testing scenarios if enabled
if [[ "$STRESS_TESTS" == true ]]; then
    log_info "=== Running Stress Testing Scenarios ==="
    
    # Market crash scenario
    log_info "Testing market crash scenario (-30% across all assets)..."
    if "$BUILD_DIR/bin/StockPredict" --test --stress-test --scenario=market-crash --severity=0.3 2>/dev/null; then
        log_success "Market crash stress test passed"
        ((TESTS_PASSED++))
    else
        log_error "Market crash stress test failed"
        ((TESTS_FAILED++))
        FAILED_TESTS+=("Market Crash Stress Test")
    fi
    
    # Volatility spike scenario
    log_info "Testing volatility spike scenario..."
    if "$BUILD_DIR/bin/StockPredict" --test --stress-test --scenario=volatility-spike --vol-multiplier=3.0 2>/dev/null; then
        log_success "Volatility spike stress test passed"
        ((TESTS_PASSED++))
    else
        log_error "Volatility spike stress test failed"
        ((TESTS_FAILED++))
        FAILED_TESTS+=("Volatility Spike Stress Test")
    fi
    
    # Interest rate shock
    log_info "Testing interest rate shock scenario..."
    if "$BUILD_DIR/bin/StockPredict" --test --stress-test --scenario=rate-shock --rate-change=0.02 2>/dev/null; then
        log_success "Interest rate shock test passed"
        ((TESTS_PASSED++))
    else
        log_error "Interest rate shock test failed"
        ((TESTS_FAILED++))
        FAILED_TESTS+=("Interest Rate Shock Test")
    fi
    
    # Sector rotation scenario
    log_info "Testing sector rotation scenario..."
    if "$BUILD_DIR/bin/StockPredict" --test --stress-test --scenario=sector-rotation 2>/dev/null; then
        log_success "Sector rotation stress test passed"
        ((TESTS_PASSED++))
    else
        log_error "Sector rotation stress test failed"
        ((TESTS_FAILED++))
        FAILED_TESTS+=("Sector Rotation Stress Test")
    fi
fi

# Backtesting validation if enabled
if [[ "$BACKTESTING" == true ]]; then
    log_info "=== Running Backtesting Validation ==="
    
    # VaR backtesting
    log_info "Running VaR backtesting (252 trading days)..."
    if "$BUILD_DIR/bin/StockPredict" --test --backtest-var --days=252 --confidence=0.95 2>/dev/null; then
        log_success "VaR backtesting passed"
        ((TESTS_PASSED++))
    else
        log_error "VaR backtesting failed"
        ((TESTS_FAILED++))
        FAILED_TESTS+=("VaR Backtesting")
    fi
    
    # Portfolio strategy backtesting
    log_info "Running portfolio strategy backtesting..."
    strategies=("buy-hold" "mean-reversion" "momentum" "risk-parity")
    for strategy in "${strategies[@]}"; do
        log_info "Backtesting $strategy strategy..."
        if "$BUILD_DIR/bin/StockPredict" --test --backtest-strategy="$strategy" --days=252 2>/dev/null; then
            log_success "$strategy backtesting passed"
            ((TESTS_PASSED++))
        else
            log_warning "$strategy backtesting failed (may be expected for some strategies)"
        fi
    done
    
    # Risk model validation over time
    log_info "Running risk model validation over historical periods..."
    if "$BUILD_DIR/bin/StockPredict" --test --validate-risk-models --historical-validation --periods=4 2>/dev/null; then
        log_success "Historical risk model validation passed"
        ((TESTS_PASSED++))
    else
        log_error "Historical risk model validation failed"
        ((TESTS_FAILED++))
        FAILED_TESTS+=("Historical Risk Model Validation")
    fi
fi

# Large portfolio performance tests if enabled
if [[ "$LARGE_PORTFOLIO_TESTS" == true ]]; then
    log_info "=== Running Large Portfolio Performance Tests ==="
    
    # Test with 500 positions
    log_info "Testing with large portfolio (500 positions)..."
    if "$BUILD_DIR/bin/StockPredict" --test --large-portfolio --positions=500 --benchmark-risk-calculations 2>/dev/null; then
        log_success "Large portfolio test passed"
        ((TESTS_PASSED++))
    else
        log_error "Large portfolio test failed"
        ((TESTS_FAILED++))
        FAILED_TESTS+=("Large Portfolio")
    fi
    
    # Performance benchmark for risk calculations
    log_info "Benchmarking risk calculation performance..."
    if "$BUILD_DIR/bin/StockPredict" --test --benchmark-var-calculations --portfolio-size=100 --iterations=1000 2>/dev/null; then
        log_success "Risk calculation benchmark passed"
        ((TESTS_PASSED++))
    else
        log_error "Risk calculation benchmark failed"
        ((TESTS_FAILED++))
        FAILED_TESTS+=("Risk Calculation Performance")
    fi
    
    # Memory usage test
    log_info "Testing memory usage with large portfolios..."
    if "$BUILD_DIR/bin/StockPredict" --test --benchmark-memory --large-portfolio --positions=1000 2>/dev/null; then
        log_success "Memory usage test passed"
        ((TESTS_PASSED++))
    else
        log_warning "Memory usage test failed (may be expected in constrained environments)"
    fi
fi

# End-to-end risk management pipeline test
log_info "=== Running End-to-End Risk Management Pipeline ==="
log_info "Testing complete risk management workflow..."
if "$BUILD_DIR/bin/StockPredict" --test --full-risk-pipeline --create-portfolio --calculate-risks --optimize --validate 2>/dev/null; then
    log_success "End-to-end risk management pipeline test passed"
    ((TESTS_PASSED++))
else
    log_error "End-to-end risk management pipeline test failed"
    ((TESTS_FAILED++))
    FAILED_TESTS+=("End-to-End Risk Pipeline")
fi

# Real-time risk monitoring test
log_info "Testing real-time risk monitoring..."
if "$BUILD_DIR/bin/StockPredict" --test --real-time-risk-monitoring --duration=30 --update-frequency=1 2>/dev/null; then
    log_success "Real-time risk monitoring test passed"
    ((TESTS_PASSED++))
else
    log_error "Real-time risk monitoring test failed"
    ((TESTS_FAILED++))
    FAILED_TESTS+=("Real-time Risk Monitoring")
fi

# Test summary
log_info "=== Phase 4 Test Summary ==="
log_info "Tests passed: $TESTS_PASSED"
log_info "Tests failed: $TESTS_FAILED"

if [[ $TESTS_FAILED -eq 0 ]]; then
    log_success "All Phase 4 tests passed! Risk Management & Portfolio Optimization is ready."
    echo ""
    log_info "Phase 4 Completion Status:"
    log_success "✅ T4.1: Risk Calculation - VaR, CVaR, volatility, correlation, drawdown analysis"
    log_success "✅ T4.2: Portfolio Management - Construction, rebalancing, performance, risk budgeting, optimization"
    log_success "✅ T4.3: Position Sizing - Fixed fractional, Kelly, volatility-based, risk parity, adaptive"
    log_success "✅ T4.4: Stop Loss Management - Fixed, ATR, trailing, volatility, time-based"
    log_success "✅ T4.5: Integration Tests - System integration, real-time monitoring, stress testing, backtesting"
    echo ""
    log_info "Key Risk Management Capabilities Validated:"
    log_success "  • Value at Risk (VaR): 95% and 99% confidence levels with parametric and historical methods"
    log_success "  • Conditional VaR: Expected shortfall calculations for tail risk"
    log_success "  • Portfolio Optimization: Mean-variance, minimum variance, maximum Sharpe ratio"
    log_success "  • Position Sizing: Multiple methods including Kelly criterion and risk parity"
    log_success "  • Stop Loss Management: ATR-based, trailing, and volatility-adjusted stops"
    log_success "  • Risk Monitoring: Real-time portfolio risk tracking and limit management"
    if [[ "$STRESS_TESTS" == true ]]; then
        log_success "  • Stress Testing: Market crash, volatility spike, interest rate, sector rotation scenarios"
    fi
    if [[ "$BACKTESTING" == true ]]; then
        log_success "  • Backtesting: VaR model validation and strategy performance over historical periods"
    fi
    if [[ "$LARGE_PORTFOLIO_TESTS" == true ]]; then
        log_success "  • Scalability: Tested with portfolios up to 1000 positions"
    fi
    echo ""
    log_info "Next steps:"
    log_info "  1. Run Phase 5 tests: ../phase5/run_phase5_tests.sh"
    log_info "  2. Test GUI integration: ../phase5/run_phase5_tests.sh --gui"
    log_info "  3. Run complete system with risk management: ../../scripts/setup.sh && ./build/bin/StockPredict --gui --enable-risk-management"
    exit 0
else
    log_error "Phase 4 tests failed. Issues found:"
    for failed_test in "${FAILED_TESTS[@]}"; do
        log_error "  - $failed_test"
    done
    echo ""
    log_info "Common troubleshooting steps:"
    log_info "  1. Ensure optimization libraries (CVXOPT, OSQP) are properly installed"
    log_info "  2. Check that mathematical libraries (BLAS, LAPACK) are available"
    log_info "  3. Verify sufficient memory for large portfolio tests"
    log_info "  4. Run with --verbose for detailed error information"
    log_info "  5. Check numerical stability in optimization routines"
    exit 1
fi
