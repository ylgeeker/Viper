#!/bin/bash
# Viper Project Automatic Build Script (main logic)

set -e  # Exit immediately on error

# Color definitions
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Print colored messages
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Detect CPU core count
if command -v nproc &> /dev/null; then
    NPROC=$(nproc)
elif command -v sysctl &> /dev/null; then
    NPROC=$(sysctl -n hw.ncpu)
else
    NPROC=4
fi

print_info "Detected ${NPROC} CPU cores"

# Parse command line arguments
BUILD_DIR="build"
INSTALL_DEPS=true
BUILD_TYPE="Release"
CLEAN_BUILD=false
DEPS_PREFIX="/usr/local"

while [[ $# -gt 0 ]]; do
    case $1 in
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --no-deps)
            INSTALL_DEPS=false
            shift
            ;;
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --deps-prefix)
            DEPS_PREFIX="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  --build-dir DIR      Specify build directory (default: build)"
            echo "  --no-deps            Skip dependency installation"
            echo "  --debug              Compile in Debug mode"
            echo "  --clean              Clean and rebuild"
            echo "  --deps-prefix DIR    Dependency install directory (default: /usr/local)"
            echo "  --help               Show this help message"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Resolve project root (parent of script directory)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
cd "${PROJECT_ROOT}"

# Clean build directory
if [ "$CLEAN_BUILD" = true ]; then
    print_warn "Cleaning build directory: ${BUILD_DIR}"
    rm -rf "${BUILD_DIR}"
fi

# Create build directory
print_info "Creating build directory: ${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# First configuration - detect dependencies
print_info "Configuring project (detecting dependencies)..."
cmake .. \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DDEPS_INSTALL_PREFIX=${DEPS_PREFIX} \
    -DCMAKE_BUILD_PARALLEL_LEVEL=${NPROC}

# Check if dependencies need to be installed
if [ "$INSTALL_DEPS" = true ] && grep -q "install_dependencies" Makefile 2>/dev/null; then
    print_info "Detected missing dependencies, starting installation..."
    
    # Check permissions
    if [ ! -w "${DEPS_PREFIX}" ]; then
        print_warn "Need sudo permission to install dependencies to ${DEPS_PREFIX}"
        sudo cmake --build . --target install_dependencies -- -j${NPROC}
    else
        cmake --build . --target install_dependencies -- -j${NPROC}
    fi
    
    print_info "Dependencies installed, reconfiguring project..."
    cmake ..
else
    print_info "All dependencies are ready"
fi

# Compile project
print_info "Starting project compilation..."
cmake --build . -j${NPROC}

# Compilation complete
print_info "=========================================="
print_info "Compilation complete!"
print_info "=========================================="
print_info "Executable locations:"
print_info "  - Viper: ${BUILD_DIR}/bin/viper"
print_info "  - Controller: ${BUILD_DIR}/bin/controller"
print_info "  - Transfer: ${BUILD_DIR}/bin/transfer"
print_info ""
print_info "Run tests: cd ${BUILD_DIR} && ctest"
print_info "Install: cd ${BUILD_DIR} && sudo make install"
