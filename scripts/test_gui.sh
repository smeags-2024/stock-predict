#!/bin/bash

# GUI Testing Script for Stock Prediction System
# This script tests the GUI application in various configurations

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
BUILD_DIR="build"
TEST_TIMEOUT=30
DISPLAY_NUM=":99"
GUI_EXECUTABLE=""

# Function to display help
show_help() {
    echo "GUI Testing Script for Stock Prediction System"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  --build-dir DIR       Build directory [default: build]"
    echo "  --timeout SECONDS     Test timeout [default: 30]"
    echo "  --display NUM         X11 display number [default: :99]"
    echo "  --help               Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                    # Run basic GUI tests"
    echo "  $0 --timeout 60       # Run with 60-second timeout"
    echo "  $0 --build-dir debug  # Use debug build directory"
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --timeout)
            TEST_TIMEOUT="$2"
            shift 2
            ;;
        --display)
            DISPLAY_NUM="$2"
            shift 2
            ;;
        --help)
            show_help
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# Check if build directory exists
if [[ ! -d "$BUILD_DIR" ]]; then
    log_error "Build directory '$BUILD_DIR' does not exist"
    log_info "Please run the build script first: ./scripts/setup.sh"
    exit 1
fi

# Find GUI executable
GUI_EXECUTABLE="$BUILD_DIR/bin/StockPredict_gui"
if [[ ! -f "$GUI_EXECUTABLE" ]]; then
    log_error "GUI executable not found at: $GUI_EXECUTABLE"
    log_info "Please ensure the project is built with GUI support"
    exit 1
fi

log_info "Starting GUI tests for Stock Prediction System..."
log_info "Build directory: $BUILD_DIR"
log_info "Test timeout: ${TEST_TIMEOUT}s"
log_info "Display: $DISPLAY_NUM"

# Function to start Xvfb
start_xvfb() {
    log_info "Starting virtual X11 display..."
    
    # Kill any existing Xvfb on this display
    pkill -f "Xvfb $DISPLAY_NUM" || true
    sleep 1
    
    # Start Xvfb
    Xvfb $DISPLAY_NUM -screen 0 1920x1080x24 -ac +extension GLX +render -noreset &
    XVFB_PID=$!
    
    # Wait for Xvfb to start
    sleep 2
    
    # Check if Xvfb is running
    if ! kill -0 $XVFB_PID 2>/dev/null; then
        log_error "Failed to start Xvfb"
        return 1
    fi
    
    export DISPLAY=$DISPLAY_NUM
    log_success "Virtual display started (PID: $XVFB_PID)"
    return 0
}

# Function to stop Xvfb
stop_xvfb() {
    if [[ -n "${XVFB_PID:-}" ]]; then
        log_info "Stopping virtual display..."
        kill $XVFB_PID 2>/dev/null || true
        wait $XVFB_PID 2>/dev/null || true
        log_success "Virtual display stopped"
    fi
}

# Function to test GUI startup
test_gui_startup() {
    log_info "Testing GUI application startup..."
    
    # Start the GUI application in test mode
    timeout $TEST_TIMEOUT $GUI_EXECUTABLE --test-mode &
    GUI_PID=$!
    
    # Wait a bit for the application to start
    sleep 5
    
    # Check if the application is still running
    if kill -0 $GUI_PID 2>/dev/null; then
        log_success "GUI application started successfully"
        
        # Let it run for a few more seconds
        sleep 3
        
        # Gracefully terminate
        kill -TERM $GUI_PID 2>/dev/null || true
        wait $GUI_PID 2>/dev/null || true
        
        log_success "GUI startup test passed"
        return 0
    else
        log_error "GUI application failed to start or crashed"
        return 1
    fi
}

# Function to test GUI components
test_gui_components() {
    log_info "Testing GUI component initialization..."
    
    # This would be expanded with actual Qt testing framework
    # For now, we'll just verify the application can load without crashing
    
    timeout $TEST_TIMEOUT $GUI_EXECUTABLE --validate-components &
    GUI_PID=$!
    
    sleep 3
    
    if kill -0 $GUI_PID 2>/dev/null; then
        kill -TERM $GUI_PID 2>/dev/null || true
        wait $GUI_PID 2>/dev/null || true
        log_success "GUI components test passed"
        return 0
    else
        log_error "GUI components test failed"
        return 1
    fi
}

# Function to test GUI with different themes
test_gui_themes() {
    log_info "Testing GUI with different themes..."
    
    # Test light theme
    log_info "Testing light theme..."
    timeout $TEST_TIMEOUT $GUI_EXECUTABLE --theme=light --test-mode &
    GUI_PID=$!
    sleep 3
    
    if kill -0 $GUI_PID 2>/dev/null; then
        kill -TERM $GUI_PID 2>/dev/null || true
        wait $GUI_PID 2>/dev/null || true
        log_success "Light theme test passed"
    else
        log_error "Light theme test failed"
        return 1
    fi
    
    # Test dark theme
    log_info "Testing dark theme..."
    timeout $TEST_TIMEOUT $GUI_EXECUTABLE --theme=dark --test-mode &
    GUI_PID=$!
    sleep 3
    
    if kill -0 $GUI_PID 2>/dev/null; then
        kill -TERM $GUI_PID 2>/dev/null || true
        wait $GUI_PID 2>/dev/null || true
        log_success "Dark theme test passed"
    else
        log_error "Dark theme test failed"
        return 1
    fi
    
    return 0
}

# Function to run performance tests
test_gui_performance() {
    log_info "Testing GUI performance..."
    
    # Test with performance monitoring
    timeout $TEST_TIMEOUT $GUI_EXECUTABLE --benchmark-mode &
    GUI_PID=$!
    
    sleep 10  # Let it run longer for performance measurement
    
    if kill -0 $GUI_PID 2>/dev/null; then
        kill -TERM $GUI_PID 2>/dev/null || true
        wait $GUI_PID 2>/dev/null || true
        log_success "GUI performance test completed"
        return 0
    else
        log_error "GUI performance test failed"
        return 1
    fi
}

# Main test execution
main() {
    local exit_code=0
    
    # Trap to ensure cleanup
    trap 'stop_xvfb; exit $exit_code' EXIT INT TERM
    
    # Start virtual display
    if ! start_xvfb; then
        log_error "Failed to start virtual display"
        exit_code=1
        return
    fi
    
    # Run tests
    log_info "Running GUI test suite..."
    
    if ! test_gui_startup; then
        log_error "GUI startup test failed"
        exit_code=1
    fi
    
    if ! test_gui_components; then
        log_error "GUI components test failed"
        exit_code=1
    fi
    
    if ! test_gui_themes; then
        log_error "GUI themes test failed"
        exit_code=1
    fi
    
    if ! test_gui_performance; then
        log_error "GUI performance test failed"
        exit_code=1
    fi
    
    if [[ $exit_code -eq 0 ]]; then
        log_success "All GUI tests passed!"
        log_info "GUI application is ready for deployment"
    else
        log_error "Some GUI tests failed"
        log_info "Please check the application logs for more details"
    fi
}

# Check dependencies
check_dependencies() {
    local missing_deps=()
    
    if ! command -v Xvfb &> /dev/null; then
        missing_deps+=("xvfb")
    fi
    
    if ! command -v timeout &> /dev/null; then
        missing_deps+=("timeout")
    fi
    
    if [[ ${#missing_deps[@]} -gt 0 ]]; then
        log_error "Missing dependencies: ${missing_deps[*]}"
        log_info "Install with: sudo apt-get install xvfb coreutils"
        return 1
    fi
    
    return 0
}

# Verify Qt environment
verify_qt_environment() {
    log_info "Verifying Qt environment..."
    
    # Check if Qt libraries are available
    if ldd "$GUI_EXECUTABLE" | grep -q "not found"; then
        log_error "Missing Qt libraries detected"
        log_info "Run 'ldd $GUI_EXECUTABLE' to see missing dependencies"
        return 1
    fi
    
    log_success "Qt environment verified"
    return 0
}

# Entry point
if check_dependencies && verify_qt_environment; then
    main
else
    log_error "Environment verification failed"
    exit 1
fi
