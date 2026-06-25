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

cmake_minimum_required (VERSION 3.12)

project (cachelib-hybrid-cache-example VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(folly REQUIRED)
find_package(gflags REQUIRED)
find_package(glog REQUIRED)

set(CACHELIB_ROOT "${CMAKE_SOURCE_DIR}/../../build-cachelib")

include_directories(${CMAKE_SOURCE_DIR}/../../)
include_directories(${CMAKE_SOURCE_DIR}/../../cachelib)
include_directories(${CACHELIB_ROOT}/cachelib)

add_definitions(-DGLOG_USE_GLOG_EXPORT)

add_executable(hybrid-cache-example main.cpp)

target_link_libraries(hybrid-cache-example 
    ${Boost_LIBRARIES}
    folly
    gflags
    glog
    ${CACHELIB_ROOT}/allocator/libcachelib_allocator.a
    ${CACHELIB_ROOT}/allocator/libcachelib_nvmitem.a
    ${CACHELIB_ROOT}/navy/libcachelib_navy.a
    ${CACHELIB_ROOT}/shm/libcachelib_shm.a
    ${CACHELIB_ROOT}/datatype/libcachelib_datatype.a
    ${CACHELIB_ROOT}/common/libcachelib_common.a
    Threads::Threads
)