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
            echo "Options:"
            echo "  --build-type TYPE    Build type (Debug/Release) [default: Release]"
            echo "  --no-deps           Skip dependency installation"
            echo "  --cuda              Enable CUDA support"
            echo "  --no-tests          Skip testing"
            echo "  --jobs NUM          Number of parallel jobs [default: nproc]"
            echo "  --build-dir DIR     Build directory [default: build]"
            echo "  --help              Show this help message"
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

log_info "Starting Stock Predict build setup..."
log_info "Build type: $BUILD_TYPE"
log_info "Parallel jobs: $JOBS"
log_info "Build directory: $BUILD_DIR"

# Check if we're on a supported system
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    DISTRO=$(lsb_release -si 2>/dev/null || echo "Unknown")
    log_info "Detected Linux distribution: $DISTRO"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    log_info "Detected macOS"
    DISTRO="macOS"
else
    log_error "Unsupported operating system: $OSTYPE"
    exit 1
fi

# Function to install system dependencies
install_system_deps() {
    log_info "Installing system dependencies..."
    
    if [[ "$DISTRO" == "Ubuntu" ]] || [[ "$DISTRO" == "Debian" ]]; then
        sudo apt-get update
        sudo apt-get install -y \
            build-essential \
            cmake \
            ninja-build \
            pkg-config \
            python3 \
            python3-pip \
            python3-venv \
            git \
            wget \
            curl \
            libssl-dev \
            libblas-dev \
            liblapack-dev \
            libhdf5-dev \
            libeigen3-dev \
            qt6-base-dev \
            qt6-charts-dev \
            libgl1-mesa-dev \
            xvfb \
            libxkbcommon-x11-0 \
            libxcb-icccm4 \
            libxcb-image0 \
            libxcb-keysyms1 \
            libxcb-randr0 \
            libxcb-render-util0 \
            libxcb-xinerama0 \
            libxcb-xfixes0
        
        # Install CUDA if requested
        if [[ "$ENABLE_CUDA" == true ]]; then
            log_info "Installing CUDA support..."
            wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/cuda-keyring_1.0-1_all.deb
            sudo dpkg -i cuda-keyring_1.0-1_all.deb
            sudo apt-get update
            sudo apt-get -y install cuda-toolkit-11-8
        fi
        
    elif [[ "$DISTRO" == "Fedora" ]] || [[ "$DISTRO" == "CentOS" ]] || [[ "$DISTRO" == "RedHat" ]]; then
        sudo dnf update -y
        sudo dnf groupinstall -y "Development Tools"
        sudo dnf install -y \
            cmake \
            ninja-build \
            python3 \
            python3-pip \
            git \
            wget \
            curl \
            openssl-devel \
            blas-devel \
            lapack-devel \
            hdf5-devel \
            eigen3-devel \
            qt6-qtbase-devel \
            qt6-qtcharts-devel \
            mesa-libGL-devel \
            xorg-x11-server-Xvfb
            
    elif [[ "$DISTRO" == "macOS" ]]; then
        # Check if Homebrew is installed
        if ! command -v brew &> /dev/null; then
            log_error "Homebrew is required on macOS. Please install it first:"
            log_error "  /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
            exit 1
        fi
        
        brew update
        brew install \
            cmake \
            ninja \
            python3 \
            pkg-config \
            openssl \
            hdf5 \
            eigen
    else
        log_warning "Unknown distribution. Please install dependencies manually:"
        log_warning "  - C++20 compatible compiler (GCC 10+, Clang 12+)"
        log_warning "  - CMake 3.20+"
        log_warning "  - Python 3.8+"
        log_warning "  - pkg-config"
        log_warning "  - OpenSSL development headers"
        log_warning "  - BLAS/LAPACK libraries"
        log_warning "  - HDF5 development headers"
        log_warning "  - Eigen3 library"
    fi
}

# Function to setup Python environment and install Conan
setup_python_env() {
    log_info "Setting up Python environment..."
    
    # Create virtual environment if it doesn't exist
    if [[ ! -d "venv" ]]; then
        python3 -m venv venv
    fi
    
    # Activate virtual environment
    source venv/bin/activate
    
    # Upgrade pip
    pip install --upgrade pip
    
    # Install Conan
    log_info "Installing Conan package manager..."
    pip install "conan>=2.0"
    
    # Setup Conan profile
    conan profile detect --force
    
    log_success "Python environment setup complete"
}

# Function to install dependencies with Conan
install_conan_deps() {
    log_info "Installing C++ dependencies with Conan..."
    
    # Activate virtual environment
    source venv/bin/activate
    
    # Create build directory
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    # Configure Conan options
    CONAN_OPTIONS=""
    if [[ "$ENABLE_CUDA" == true ]]; then
        CONAN_OPTIONS="$CONAN_OPTIONS -o libtorch/*:cuda=True"
    fi
    
    # Install dependencies
    conan install .. \
        --build=missing \
        -s build_type="$BUILD_TYPE" \
        $CONAN_OPTIONS
    
    cd ..
    log_success "Dependencies installed successfully"
}

# Function to configure and build the project
build_project() {
    log_info "Configuring and building project..."
    
    cd "$BUILD_DIR"
    
    # Configure with CMake
    CMAKE_OPTIONS=(
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
        -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
        -G Ninja
    )
    
    if [[ "$ENABLE_TESTING" == true ]]; then
        CMAKE_OPTIONS+=(-DBUILD_TESTING=ON)
    fi
    
    if [[ "$ENABLE_CUDA" == true ]]; then
        CMAKE_OPTIONS+=(-DENABLE_CUDA=ON)
    fi
    
    cmake .. "${CMAKE_OPTIONS[@]}"
    
    # Build
    log_info "Building with $JOBS parallel jobs..."
    ninja -j"$JOBS"
    
    cd ..
    log_success "Build completed successfully"
}

# Function to run tests
run_tests() {
    if [[ "$ENABLE_TESTING" == true ]]; then
        log_info "Running tests..."
        cd "$BUILD_DIR"
        
        # Run unit tests
        if [[ -f "bin/run_tests" ]]; then
            ./bin/run_tests
        else
            ctest --output-on-failure --parallel "$JOBS"
        fi
        
        cd ..
        log_success "All tests passed"
    else
        log_info "Testing disabled"
    fi
}

# Function to check build artifacts
check_artifacts() {
    log_info "Checking build artifacts..."
    
    if [[ -f "$BUILD_DIR/bin/StockPredict" ]]; then
        log_success "Main executable created: $BUILD_DIR/bin/StockPredict"
    else
        log_error "Main executable not found!"
        exit 1
    fi
    
    if [[ -f "$BUILD_DIR/lib/libStockPredict_lib.a" ]] || [[ -f "$BUILD_DIR/lib/libStockPredict_lib.so" ]]; then
        log_success "Library created successfully"
    else
        log_warning "Library not found (this might be expected)"
    fi
}

# Function to display next steps
show_next_steps() {
    log_success "Setup completed successfully!"
    echo
    log_info "Next steps:"
    echo "  1. Run the application:"
    echo "     cd $BUILD_DIR && ./bin/StockPredict --help"
    echo
    echo "  2. Run tests (if enabled):"
    echo "     cd $BUILD_DIR && ctest"
    echo
    echo "  3. Run benchmarks:"
    echo "     cd $BUILD_DIR && ./bin/run_benchmarks"
    echo
    echo "  4. Activate Python environment for development:"
    echo "     source venv/bin/activate"
    echo
    log_info "For more information, see the README.md file"
}

# Main execution
main() {
    # Install system dependencies
    if [[ "$INSTALL_DEPS" == true ]]; then
        install_system_deps
    fi
    
    # Setup Python environment
    setup_python_env
    
    # Install Conan dependencies
    install_conan_deps
    
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
