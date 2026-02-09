#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Default build dir for quick local builds.
BUILD_DIR="build_release"

# Create build directory if it doesn't exist
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Run CMake
echo "Running CMake..."
if ! cmake ..; then
    echo -e "${RED}CMake configuration failed. Exiting.${NC}"
    exit 1
fi

# Run make
echo "Building project..."
if ! make; then
    echo -e "${RED}Build failed. Exiting.${NC}"
    exit 1
fi
