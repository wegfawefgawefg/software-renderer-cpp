#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Default build type and Valgrind flag
BUILD_TYPE="Release"
BUILD_DIR="build_release"
USE_VALGRIND=false

# Parse arguments
for arg in "$@"
do
    case $arg in
        --dev)
            # Closest CMake equivalent: optimized with debug symbols.
            BUILD_TYPE="RelWithDebInfo"
            shift
            ;;
        --debug)
            BUILD_TYPE="Debug"
            BUILD_DIR="build_debug"
            shift
            ;;
        --valgrind)
            USE_VALGRIND=true
            BUILD_TYPE="Debug"
            BUILD_DIR="build_debug"
            shift
            ;;
        --help|-h)
            echo "Usage: $0 [--dev] [--debug] [--valgrind]"
            echo "  --dev       Build RelWithDebInfo (optimized + debug symbols)"
            echo "  --debug     Build Debug (separate build dir)"
            echo "  --valgrind  Run under Valgrind (implies Debug)"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $arg${NC}"
            echo "Use --help or -h for usage information."
            exit 1
            ;;
    esac
done

# Create build directory if it doesn't exist
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Run CMake with the specified build type
echo "Running CMake with build type: ${BUILD_TYPE}..."
if ! cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ..; then
    echo -e "${RED}CMake configuration failed. Exiting.${NC}"
    exit 1
fi

# Run make (using all available cores)
echo "Building project..."
if ! make -j$(nproc); then
    echo -e "${RED}Build failed. Exiting.${NC}"
    exit 1
fi

# If we've made it this far, the build was successful
echo -e "${GREEN}Build successful.${NC}"
cd ..

# Run the executable
if [ "$USE_VALGRIND" = true ]; then
    echo "Running Software Renderer with Valgrind..."
    valgrind --leak-check=full --track-origins=yes "./${BUILD_DIR}/renderer"
else
    echo "Running Software Renderer..."
    "./${BUILD_DIR}/renderer"
fi
