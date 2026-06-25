/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cachelib/allocator/CacheAllocator.h>
#include <iostream>
#include <cassert>
#include <memory>

using Cache = facebook::cachelib::LruAllocator;
using PoolId = facebook::cachelib::PoolId;

std::unique_ptr<Cache> gCache_;
PoolId defaultPool_;

void initializeCache() {
  Cache::Config config;
  config
      .setCacheSize(1 * 1024 * 1024 * 1024) // 1 GB
      .setCacheName("My cache")
      .setAccessConfig({25, 10})
      .validate();
  gCache_ = std::make_unique<Cache>(config);
  defaultPool_ = gCache_->addPool("default", gCache_->getCacheMemoryStats().ramCacheSize);
}

void destroyCache() {
  gCache_.reset();
}

int main(int argc, char** argv) {
  initializeCache();

  // Insert an item
  std::string key = "key";
  std::string value = "value";
  
  auto handle = gCache_->allocate(defaultPool_, key, value.size());
  if (handle) {
    std::memcpy(handle->getMemory(), value.data(), value.size());
    gCache_->insertOrReplace(std::move(handle));
  }

  // Read the item back
  auto readHandle = gCache_->find(key);
  if (readHandle) {
    std::string sp(reinterpret_cast<const char*>(readHandle->getMemory()), readHandle->getSize());
    assert(sp == value);
    
    // Print the value
    std::cout << "value = " << sp << '\n';
    
    // Compare cache size with default pool size
    auto cache_size = 1024 * 1024 * 1024; // 1 GB
    auto default_pool_size = gCache_->getCacheMemoryStats().ramCacheSize;
    std::cout << "cache size = " << cache_size << '\n';
    std::cout << "default pool size = " << default_pool_size << '\n';
  }

  destroyCache();
  return 0;
}