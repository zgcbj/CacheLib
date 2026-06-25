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

# Root directory for the CacheLib project
CLBASE="$PWD/../.."

# Additional "FindXXX.cmake" files are here (e.g. FindSodium.cmake)
CLCMAKE="$CLBASE/cachelib/cmake"

# Create a local build using the source tree directly
mkdir -p build
cd build

# Build using the source tree directly instead of installed package
cmake .. \
  -DCMAKE_MODULE_PATH="$CLCMAKE" \
  -DCMAKE_BUILD_TYPE=Debug \
  -Dcachelib_SOURCE_DIR="$CLBASE" \
  -Dcachelib_INCLUDE_DIR="$CLBASE" \
  -Dcachelib_LIB_DIR="$CLBASE/build-cachelib" \
  -DCMAKE_CXX_STANDARD=20

make