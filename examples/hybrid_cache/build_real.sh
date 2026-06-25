#!/bin/sh
# Build script for real CacheLib hybrid cache example

set -e

echo "Building real hybrid-cache-example with CacheLib..."

# Check if we're on macOS
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo "This script is designed for macOS"
    exit 1
fi

# Get Homebrew prefix
HOMEBREW_PREFIX=$(brew --prefix)

# Check required dependencies
DEPS="cmake boost folly gflags glog"
for dep in $DEPS; do
    if ! brew list | grep -q "^$dep$"; then
        echo "Installing $dep..."
        brew install $dep
    fi
done

# Create build directory
mkdir -p build
cd build

# Try to build with the actual CacheLib headers
echo "Configuring CMake..."

# Create a CMakeLists.txt that works with the source tree
cat > CMakeLists.txt << EOF
cmake_minimum_required(VERSION 3.12)
project(hybrid-cache-example)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(folly REQUIRED)
find_package(gflags REQUIRED)
find_package(glog REQUIRED)

# Include directories from source tree
include_directories(../../)
include_directories(../../cachelib)

# Add executable
add_executable(hybrid-cache-example ../main.cpp)

# Link libraries
target_link_libraries(hybrid-cache-example 
    \${Boost_LIBRARIES}
    folly
    gflags
    glog
)
EOF

# Try to configure and build
if cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="${HOMEBREW_PREFIX}" \
    -DBoost_NO_BOOST_CMAKE=ON \
    -DBOOST_ROOT="${HOMEBREW_PREFIX}" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON; then
    
    echo "CMake configuration successful, building..."
    make -j$(sysctl -n hw.ncpu)
    echo "Build successful! Run with: ./hybrid-cache-example"
else
    echo "CMake configuration failed, trying alternative approach..."
    
    # Try direct compilation with g++
    cd ..
    g++ -std=c++20 \
        -I../../ \
        -I../../cachelib \
        -I${HOMEBREW_PREFIX}/include \
        -L${HOMEBREW_PREFIX}/lib \
        -lfolly -lgflags -lglog -lboost_system -lboost_filesystem \
        -pthread \
        main.cpp -o hybrid-cache-example-real 2>/dev/null || {
        echo "Direct compilation also failed. Please ensure all dependencies are properly installed."
        echo "Try: brew install boost folly gflags glog"
        exit 1
    }
    
    echo "Direct compilation successful! Run with: ./hybrid-cache-example-real"
fi