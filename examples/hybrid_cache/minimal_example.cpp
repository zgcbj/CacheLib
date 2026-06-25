#include <iostream>
#include <string>
#include <memory>
#include <cassert>
#include <cstring>

// Mock the CacheLib interface for demonstration
class MockCache {
public:
    struct Config {
        size_t cacheSize = 1024 * 1024 * 1024; // 1GB
        std::string cacheName = "mock-cache";
        
        Config& setCacheSize(size_t size) { cacheSize = size; return *this; }
        Config& setCacheName(const std::string& name) { cacheName = name; return *this; }
        Config& setAccessConfig(std::pair<int, int>) { return *this; }
        void validate() {}
    };
    
    struct Item {
        char* data;
        size_t size;
        Item(size_t s) : size(s) { data = new char[s]; }
        ~Item() { delete[] data; }
        void* getMemory() { return data; }
        size_t getSize() const { return size; }
    };
    
    using WriteHandle = std::unique_ptr<Item>;
    
    MockCache(const Config& config) {
        std::cout << "[Mock] Created cache: " << config.cacheName 
                  << " size: " << config.cacheSize << " bytes" << std::endl;
    }
    
    WriteHandle allocate(int, const std::string& key, size_t size) {
        std::cout << "[Mock] Allocating item for key: " << key << " size: " << size << std::endl;
        return std::make_unique<Item>(size);
    }
    
    void insertOrReplace(WriteHandle handle) {
        std::cout << "[Mock] Inserting item" << std::endl;
    }
    
    WriteHandle find(const std::string& key) {
        std::cout << "[Mock] Finding item for key: " << key << std::endl;
        // Return nullptr to simulate cache miss for demo
        return nullptr;
    }
};

int main(int argc, char** argv) {
    std::cout << "=== Hybrid Cache Demo (Mock Version) ===" << std::endl;
    
    // Create cache
    MockCache::Config config;
    config.setCacheSize(1024 * 1024 * 1024) // 1 GB
          .setCacheName("hybrid-cache-demo")
          .validate();
    
    MockCache cache(config);
    
    // Test put/get
    std::string key = "test_key";
    std::string value = "test_value";
    
    // Put item
    auto handle = cache.allocate(0, key, value.size());
    if (handle) {
        std::memcpy(handle->getMemory(), value.data(), value.size());
        cache.insertOrReplace(std::move(handle));
        std::cout << "[Demo] Put key: " << key << " value: " << value << std::endl;
    }
    
    // Get item
    auto readHandle = cache.find(key);
    if (readHandle) {
        std::string result(static_cast<char*>(readHandle->getMemory()), readHandle->getSize());
        std::cout << "[Demo] Got key: " << key << " value: " << result << std::endl;
    } else {
        std::cout << "[Demo] Key not found: " << key << std::endl;
    }
    
    std::cout << "=== Demo completed ===" << std::endl;
    return 0;
}
