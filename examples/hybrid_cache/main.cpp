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

/**
 * hybrid_cache 示例
 *
 * 演示 CacheLib 的二级缓存（HybridCache）用法：
 *   - 第一级（L1）：1 GB DRAM 内存缓存
 *   - 第二级（L2）：1 GB 磁盘文件缓存（通过 Navy 引擎）
 *
 * 热数据常驻 DRAM，冷数据被淘汰后自动溢出到磁盘文件；
 * 再次访问磁盘上的数据时，CacheLib 会将其自动提升回 DRAM。
 */

#include <cachelib/allocator/CacheAllocator.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>

// 使用 LRU 淘汰策略
using Cache  = facebook::cachelib::LruAllocator;
using PoolId = facebook::cachelib::PoolId;

// 全局缓存实例和默认内存池
std::unique_ptr<Cache> gCache_;
PoolId                  gDefaultPool_;

// -----------------------------------------------------------------------
// initializeCache()
//   配置并启动带有 HybridCache（Navy）的二级缓存：
//     L1：1 GB DRAM
//     L2：1 GB 磁盘文件 /tmp/navy_cache_file
// -----------------------------------------------------------------------
void initializeCache(const std::string& nvmFilePath) {
  // ── L1（DRAM）配置 ──────────────────────────────────────────────────
  Cache::Config config;
  config
      .setCacheSize(1ULL * 1024 * 1024 * 1024) // L1：1 GB DRAM
      .setCacheName("hybrid-cache")
      .setAccessConfig({25, 10})               // hash table 桶数 & 锁数
      .validate();

  // ── L2（NVM/磁盘）配置 ────────────────────────────────────────────────
  Cache::NvmCacheConfig nvmConfig;

  // 指定磁盘文件路径及大小（1 GB），truncateFile=true 表示首次运行时截断文件
  nvmConfig.navyConfig.setSimpleFile(
      nvmFilePath,
      1ULL * 1024 * 1024 * 1024, // L2：1 GB 磁盘文件
      /*truncateFile=*/true);

  // 设备块大小（最小 IO 粒度），通常与文件系统块大小对齐
  nvmConfig.navyConfig.setBlockSize(4096);

  // BlockCache：每个 Region 大小为 16 MB（NVM 上的写入单元）
  nvmConfig.navyConfig.blockCache().setRegionSize(16 * 1024 * 1024);

  // BigHash：将 10% 的 NVM 空间用于存储小对象（< 1 KB）
  // 剩余 90% 交给 BlockCache 存储大对象
  nvmConfig.navyConfig.bigHash()
      .setSizePctAndMaxItemSize(10, 1024) // 10% 空间，最大 item 1 KB
      .setBucketSize(4096)
      .setBucketBfSize(8);               // 每个桶 8 字节 Bloom Filter

  // 将 NVM 配置附加到主缓存配置
  config.enableNvmCache(nvmConfig);

  // ── 创建缓存实例 ───────────────────────────────────────────────────────
  gCache_ = std::make_unique<Cache>(config);

  // 将全部 DRAM 内存划分为一个默认池
  gDefaultPool_ = gCache_->addPool(
      "default", gCache_->getCacheMemoryStats().ramCacheSize);

  std::cout << "[init] L1 DRAM pool size = "
            << gCache_->getCacheMemoryStats().ramCacheSize
            << " bytes\n";
  std::cout << "[init] L2 NVM  file path = " << nvmFilePath << "\n";
}

// -----------------------------------------------------------------------
// destroyCache()  —— 释放缓存实例
// -----------------------------------------------------------------------
void destroyCache() {
  gCache_.reset();
}

// -----------------------------------------------------------------------
// putItem()  —— 向缓存写入一条 key-value
// -----------------------------------------------------------------------
bool putItem(const std::string& key, const std::string& value) {
  // allocate() 在 L1 DRAM 中为 item 申请内存
  auto handle = gCache_->allocate(gDefaultPool_, key, value.size());
  if (!handle) {
    std::cerr << "[put] allocate failed for key=" << key << "\n";
    return false;
  }
  // 将用户数据拷贝到 item 内存区域
  std::memcpy(handle->getMemory(), value.data(), value.size());
  // insertOrReplace() 使 item 对其他线程可见（若 key 已存在则替换）
  gCache_->insertOrReplace(std::move(handle));
  return true;
}

// -----------------------------------------------------------------------
// getItem()  —— 从缓存读取一条 key 对应的 value
//   若 item 在 L2（NVM），find() 会阻塞直到 item 被提升回 L1 DRAM
// -----------------------------------------------------------------------
std::string getItem(const std::string& key) {
  // find() 透明地查找 L1 和 L2
  auto handle = gCache_->find(key);
  if (!handle) {
    return {}; // cache miss
  }
  // handle->getMemory() 在 item 就绪（已提升到 DRAM）后才可安全访问
  return std::string(
      reinterpret_cast<const char*>(handle->getMemory()),
      handle->getSize());
}

// -----------------------------------------------------------------------
// main()
// -----------------------------------------------------------------------
int main(int argc, char** argv) {
  // 磁盘文件路径可通过命令行参数覆盖，默认 /tmp/navy_cache_file
  std::string nvmFile = "/tmp/navy_cache_file";
  if (argc >= 2) {
    nvmFile = argv[1];
  }

  // ── 1. 初始化二级缓存 ────────────────────────────────────────────────
  initializeCache(nvmFile);

  // ── 2. 写入几条测试数据 ──────────────────────────────────────────────
  const int kItems = 5;
  for (int i = 0; i < kItems; ++i) {
    std::string key   = "key_"   + std::to_string(i);
    std::string value = "value_" + std::to_string(i);
    if (putItem(key, value)) {
      std::cout << "[put] key=" << key << "  value=" << value << "\n";
    }
  }

  // ── 3. 读取并验证 ────────────────────────────────────────────────────
  for (int i = 0; i < kItems; ++i) {
    std::string key      = "key_"   + std::to_string(i);
    std::string expected = "value_" + std::to_string(i);
    std::string got      = getItem(key);

    if (got.empty()) {
      std::cout << "[get] key=" << key << "  MISS\n";
    } else {
      assert(got == expected);
      std::cout << "[get] key=" << key << "  value=" << got << "  HIT\n";
    }
  }

  // ── 4. 测试 cache miss ───────────────────────────────────────────────
  std::string missing = getItem("nonexistent_key");
  std::cout << "[get] key=nonexistent_key  "
            << (missing.empty() ? "MISS (expected)" : "HIT (unexpected)")
            << "\n";

  // ── 5. 销毁缓存 ──────────────────────────────────────────────────────
  destroyCache();
  std::cout << "[done] cache destroyed.\n";
  return 0;
}