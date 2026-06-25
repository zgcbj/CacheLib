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
 * Simple Hybrid Cache Example
 * 
 * This is a simplified version that demonstrates the core concepts
 * of a two-level cache without requiring the full CacheLib dependency chain.
 */

#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>
#include <list>
#include <cassert>
#include <cstring>
#include <fstream>
#include <mutex>

// Simple in-memory cache implementation
class SimpleCache {
private:
    struct CacheItem {
        std::string key;
        std::string value;
        size_t size;
        bool in_memory;
        
        CacheItem(const std::string& k, const std::string& v, bool mem = true) 
            : key(k), value(v), size(v.size()), in_memory(mem) {}
    };
    
    std::unordered_map<std::string, std::unique_ptr<CacheItem>> items_;
    std::list<std::string> lru_list_;
    std::mutex mutex_;
    
    size_t memory_capacity_;
    size_t disk_capacity_;
    size_t memory_used_;
    size_t disk_used_;
    
    std::string disk_file_;

public:
    struct Config {
        size_t memorySize = 1024 * 1024 * 1024; // 1GB
        size_t diskSize = 1024 * 1024 * 1024;   // 1GB
        std::string diskFile = "/tmp/hybrid_cache.dat";
        
        Config& setMemorySize(size_t size) { memorySize = size; return *this; }
        Config& setDiskSize(size_t size) { diskSize = size; return *this; }
        Config& setDiskFile(const std::string& file) { diskFile = file; return *this; }
        void validate() {}
    };
    
    SimpleCache(const Config& config) 
        : memory_capacity_(config.memorySize)
        , disk_capacity_(config.diskSize)
        , memory_used_(0)
        , disk_used_(0)
        , disk_file_(config.diskFile) {
        
        std::cout << "[SimpleCache] Created hybrid cache" << std::endl;
        std::cout << "  Memory cache: " << memory_capacity_ / (1024*1024) << " MB" << std::endl;
        std::cout << "  Disk cache: " << disk_capacity_ / (1024*1024) << " MB" << std::endl;
        std::cout << "  Disk file: " << disk_file_ << std::endl;
        
        // Create disk file
        std::ofstream file(disk_file_, std::ios::binary | std::ios::trunc);
        if (!file) {
            std::cerr << "Warning: Could not create disk file" << std::endl;
        }
    }
    
    ~SimpleCache() {
        // Cleanup disk file
        std::remove(disk_file_.c_str());
    }
    
    bool put(const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        size_t item_size = value.size();
        
        // If item exists, remove it first
        auto it = items_.find(key);
        if (it != items_.end()) {
            removeItem(key);
        }
        
        // Try to put in memory first
        if (memory_used_ + item_size <= memory_capacity_) {
            auto item = std::make_unique<CacheItem>(key, value, true);
            items_[key] = std::move(item);
            lru_list_.push_front(key);
            memory_used_ += item_size;
            std::cout << "[PUT] Key '" << key << "' stored in memory" << std::endl;
            return true;
        }
        
        // Try to put on disk
        if (disk_used_ + item_size <= disk_capacity_) {
            auto item = std::make_unique<CacheItem>(key, value, false);
            items_[key] = std::move(item);
            disk_used_ += item_size;
            
            // Save to disk file
            saveToDisk(key, value);
            std::cout << "[PUT] Key '" << key << "' stored on disk" << std::endl;
            return true;
        }
        
        // Evict oldest item and try again
        if (!lru_list_.empty()) {
            std::string oldest_key = lru_list_.back();
            removeItem(oldest_key);
            return put(key, value); // Recursive call
        }
        
        return false;
    }
    
    std::string get(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = items_.find(key);
        if (it == items_.end()) {
            std::cout << "[GET] Key '" << key << "' not found" << std::endl;
            return "";
        }
        
        CacheItem* item = it->second.get();
        
        if (item->in_memory) {
            // Move to front of LRU list
            lru_list_.remove(key);
            lru_list_.push_front(key);
            std::cout << "[GET] Key '" << key << "' found in memory" << std::endl;
            return item->value;
        } else {
            // Load from disk (promote to memory if possible)
            std::string value = loadFromDisk(key);
            if (!value.empty() && memory_used_ + value.size() <= memory_capacity_) {
                // Promote to memory
                item->in_memory = true;
                item->value = value;
                lru_list_.push_front(key);
                memory_used_ += value.size();
                disk_used_ -= value.size();
                std::cout << "[GET] Key '" << key << "' promoted from disk to memory" << std::endl;
            } else {
                std::cout << "[GET] Key '" << key << "' loaded from disk" << std::endl;
            }
            return value;
        }
    }
    
    bool remove(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        return removeItem(key);
    }
    
private:
    bool removeItem(const std::string& key) {
        auto it = items_.find(key);
        if (it == items_.end()) {
            return false;
        }
        
        CacheItem* item = it->second.get();
        if (item->in_memory) {
            memory_used_ -= item->size;
            lru_list_.remove(key);
        } else {
            disk_used_ -= item->size;
            // Remove from disk file would be more complex in real implementation
        }
        
        items_.erase(it);
        std::cout << "[REMOVE] Key '" << key << "' removed" << std::endl;
        return true;
    }
    
    void saveToDisk(const std::string& key, const std::string& value) {
        // Simplified disk storage - in real implementation this would be more sophisticated
        std::ofstream file(disk_file_, std::ios::binary | std::ios::app);
        if (file) {
            // Simple format: key_length + key + value_length + value
            size_t key_len = key.size();
            size_t val_len = value.size();
            file.write(reinterpret_cast<const char*>(&key_len), sizeof(key_len));
            file.write(key.data(), key_len);
            file.write(reinterpret_cast<const char*>(&val_len), sizeof(val_len));
            file.write(value.data(), val_len);
        }
    }
    
    std::string loadFromDisk(const std::string& key) {
        // Simplified disk loading - in real implementation this would be more sophisticated
        std::ifstream file(disk_file_, std::ios::binary);
        if (!file) return "";
        
        while (file) {
            size_t key_len;
            if (!file.read(reinterpret_cast<char*>(&key_len), sizeof(key_len))) break;
            
            std::string stored_key(key_len, '\0');
            if (!file.read(&stored_key[0], key_len)) break;
            
            size_t val_len;
            if (!file.read(reinterpret_cast<char*>(&val_len), sizeof(val_len))) break;
            
            std::string value(val_len, '\0');
            if (!file.read(&value[0], val_len)) break;
            
            if (stored_key == key) {
                return value;
            }
        }
        
        return "";
    }
};

int main(int argc, char** argv) {
    std::cout << "=== Hybrid Cache Demo (Simple Implementation) ===" << std::endl;
    
    // Create cache with 10MB memory + 10MB disk for demo
    SimpleCache::Config config;
    config.setMemorySize(10 * 1024 * 1024)  // 10MB for demo
          .setDiskSize(10 * 1024 * 1024)    // 10MB for demo
          .setDiskFile("/tmp/hybrid_demo.dat")
          .validate();
    
    SimpleCache cache(config);
    
    // Test basic operations
    std::cout << "\n--- Testing basic operations ---" << std::endl;
    
    // Put some items
    cache.put("key1", "value1");
    cache.put("key2", "value2");
    cache.put("key3", "large_value_" + std::string(1000, 'x')); // Large item
    
    // Get items
    std::string val1 = cache.get("key1");
    std::string val2 = cache.get("key2");
    std::string val3 = cache.get("key3");
    
    // Try to get non-existent key
    std::string val4 = cache.get("nonexistent");
    
    std::cout << "\n--- Cache Statistics ---" << std::endl;
    std::cout << "Retrieved values:" << std::endl;
    std::cout << "  key1: " << (val1.empty() ? "NOT FOUND" : val1) << std::endl;
    std::cout << "  key2: " << (val2.empty() ? "NOT FOUND" : val2) << std::endl;
    std::cout << "  key3: " << (val3.empty() ? "NOT FOUND" : "Found (size: " + std::to_string(val3.size()) + ")") << std::endl;
    std::cout << "  nonexistent: " << (val4.empty() ? "NOT FOUND" : val4) << std::endl;
    
    std::cout << "\n=== Demo completed ===" << std::endl;
    return 0;
}