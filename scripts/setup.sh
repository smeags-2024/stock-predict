#!/bin/bash

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
BUILD_TYPE="Release"
INSTALL_DEPS=true
ENABLE_CUDA=false
ENABLE_TESTING=true
JOBS=$(nproc)
BUILD_DIR="build"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --build-type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        --no-deps)
            INSTALL_DEPS=false
            shift
            ;;
        --cuda)
            ENABLE_CUDA=true
            shift
            ;;
        --no-tests)
            ENABLE_TESTING=false
            shift
            ;;
        --jobs)
            JOBS="$2"
            shift 2
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --build-type TYPE    Build type (Release, Debug, RelWithDebInfo) [default: Release]"
            echo "  --no-deps            Skip installing system dependencies"
            echo "  --cuda               Enable CUDA support"
            echo "  --no-tests           Disable building tests"
            echo "  --jobs N             Number of parallel jobs [default: $(nproc)]"
            echo "  --build-dir DIR      Build directory [default: build]"
            echo "  --help               Show this help message"
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

log_info "Starting Stock Predict C++ build setup..."
log_info "Build type: $BUILD_TYPE"
log_info "Parallel jobs: $JOBS"
log_info "Build directory: $BUILD_DIR"

# Detect Linux distribution
if [ -f /etc/os-release ]; then
    . /etc/os-release
    DISTRO=$ID
    log_info "Detected Linux distribution: $DISTRO"
else
    DISTRO="unknown"
    log_warning "Cannot detect Linux distribution"
fi

# Function to install system dependencies
install_system_deps() {
    log_info "Installing C++ system dependencies..."
    
    if [[ "$DISTRO" == "ubuntu" ]] || [[ "$DISTRO" == "debian" ]] || [[ "$DISTRO" == "kali" ]]; then
        sudo apt-get update
        sudo apt-get install -y \
            build-essential \
            cmake \
            ninja-build \
            pkg-config \
            git \
            wget \
            curl \
            libssl-dev \
            libblas-dev \
            liblapack-dev \
            libeigen3-dev \
            nlohmann-json3-dev \
            libspdlog-dev \
            libfmt-dev \
            libgtest-dev \
            libgmock-dev
        
        # Optional Qt6 for GUI (install if available)
        sudo apt-get install -y qt6-base-dev qt6-charts-dev || log_warning "Qt6 not available, GUI will be disabled"
        
        # Install CUDA if requested
        if [[ "$ENABLE_CUDA" == true ]]; then
            log_info "Installing CUDA support..."
            wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/cuda-keyring_1.0-1_all.deb
            sudo dpkg -i cuda-keyring_1.0-1_all.deb
            sudo apt-get update
            sudo apt-get -y install cuda-toolkit-11-8
        fi
        
    elif [[ "$DISTRO" == "fedora" ]] || [[ "$DISTRO" == "centos" ]] || [[ "$DISTRO" == "rhel" ]]; then
        sudo dnf update -y
        sudo dnf groupinstall -y "Development Tools"
        sudo dnf install -y \
            cmake \
            ninja-build \
            pkg-config \
            git \
            wget \
            curl \
            openssl-devel \
            blas-devel \
            lapack-devel \
            eigen3-devel \
            json-devel \
            spdlog-devel \
            fmt-devel \
            gtest-devel \
            gmock-devel
            
        # Optional Qt6
        sudo dnf install -y qt6-qtbase-devel qt6-qtcharts-devel || log_warning "Qt6 not available, GUI will be disabled"
        
    elif [[ "$DISTRO" == "arch" ]] || [[ "$DISTRO" == "manjaro" ]]; then
        sudo pacman -Syu --noconfirm
        sudo pacman -S --noconfirm \
            base-devel \
            cmake \
            ninja \
            pkg-config \
            git \
            wget \
            curl \
            openssl \
            blas \
            lapack \
            eigen \
            nlohmann-json \
            spdlog \
            fmt \
            gtest \
            gmock
            
        # Optional Qt6
        sudo pacman -S --noconfirm qt6-base qt6-charts || log_warning "Qt6 not available, GUI will be disabled"
        
    else
        log_warning "Unknown distribution. Please install dependencies manually:"
        log_warning "  - C++20 compatible compiler (GCC 10+, Clang 12+)"
        log_warning "  - CMake 3.20+"
        log_warning "  - pkg-config"
        log_warning "  - OpenSSL development headers"
        log_warning "  - BLAS/LAPACK libraries"
        log_warning "  - Eigen3 library"
        log_warning "  - nlohmann-json library"
        log_warning "  - spdlog library"
        log_warning "  - fmt library"
        log_warning "  - GoogleTest library (optional)"
        log_warning "  - Qt6 development packages (optional for GUI)"
    fi
    
    log_success "System dependencies installation completed"
}

# Function to configure and build the project
build_project() {
    log_info "Configuring and building project..."
    
    # Create build directory
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    # Configure CMake options
    CMAKE_OPTIONS=(
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
        -DBUILD_TESTS="$ENABLE_TESTING"
    )
    
    if [[ "$ENABLE_CUDA" == true ]]; then
        CMAKE_OPTIONS+=(-DENABLE_CUDA=ON)
    fi
    
    # Configure with CMake
    log_info "Configuring with CMake..."
    cmake "${CMAKE_OPTIONS[@]}" ..
    
    # Build
    log_info "Building with $JOBS parallel jobs..."
    cmake --build . --config "$BUILD_TYPE" -j "$JOBS"
    
    cd ..
    log_success "Project built successfully"
}

# Function to run tests
run_tests() {
    if [[ "$ENABLE_TESTING" == true ]]; then
        log_info "Running tests..."
        cd "$BUILD_DIR"
        if command -v ctest &> /dev/null; then
            ctest --output-on-failure
            log_success "Tests completed"
        else
            log_warning "CTest not available, skipping tests"
        fi
        cd ..
    else
        log_info "Testing disabled, skipping tests"
    fi
}

# Function to check build artifacts
check_artifacts() {
    log_info "Checking build artifacts..."
    
    if [[ -f "$BUILD_DIR/StockPredict" ]]; then
        log_success "Main executable created: $BUILD_DIR/StockPredict"
    else
        log_error "Main executable not found!"
        exit 1
    fi
    
    if [[ -f "$BUILD_DIR/libStockPredict_lib.a" ]] || [[ -f "$BUILD_DIR/libStockPredict_lib.so" ]]; then
        log_success "Library created successfully"
    else
        log_warning "Library not found (this might be expected)"
    fi
}

# Function to display next steps
show_next_steps() {
    log_success "C++ setup completed successfully!"
    echo
    log_info "Next steps:"
    echo "  1. Run the application:"
    echo "     cd $BUILD_DIR && ./StockPredict --help"
    echo
    echo "  2. Run tests (if enabled):"
    echo "     cd $BUILD_DIR && ctest"
    echo
    log_info "The project is now ready for development and testing."
    log_info "No Python dependencies required - this is a pure C++ project."
}

# Main execution
main() {
    # Install system dependencies
    if [[ "$INSTALL_DEPS" == true ]]; then
        install_system_deps
    fi
    
    # Build project
    build_project
    
    # Run tests
    run_tests
    
    # Check artifacts
    check_artifacts
    
    # Show next steps
    show_next_steps
}

# Trap to handle errors
trap 'log_error "Setup failed at line $LINENO"' ERR

# Run main function
main "$@"
