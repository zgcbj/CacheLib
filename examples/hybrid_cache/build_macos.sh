#!/bin/sh
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e

echo "Building hybrid-cache-example for macOS..."

# Check if required tools are available
command -v cmake >/dev/null 2>&1 || { echo >&2 "cmake is required but not installed. Please run: brew install cmake"; exit 1; }
command -v g++ >/dev/null 2>&1 || { echo >&2 "g++ is required but not installed. Please run: xcode-select --install"; exit 1; }

# Set up environment for macOS
export CC=clang
export CXX=clang++

# Get Homebrew prefix
HOMEBREW_PREFIX=$(brew --prefix)

# Create build directory
mkdir -p build
cd build

# Configure CMake with macOS-specific settings
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_STANDARD=20 \
  -DCMAKE_PREFIX_PATH="${HOMEBREW_PREFIX}" \
  -DBoost_NO_BOOST_CMAKE=ON \
  -DBoost_NO_SYSTEM_PATHS=ON \
  -DBOOST_ROOT="${HOMEBREW_PREFIX}" \
  -DBoost_INCLUDE_DIR="${HOMEBREW_PREFIX}/include" \
  -DBoost_LIBRARY_DIR="${HOMEBREW_PREFIX}/lib" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build the project
make -j$(sysctl -n hw.ncpu)

echo "Build completed successfully!"
echo "Run with: ./hybrid-cache-example"