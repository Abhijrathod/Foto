#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include <mutex>

// Fixed-size memory pool for efficient allocation
class MemoryPool {
public:
    MemoryPool(size_t blockSize, size_t numBlocks);
    ~MemoryPool();

    void* Allocate();
    void Deallocate(void* ptr);

    size_t GetBlockSize() const { return blockSize_; }
    size_t GetAvailableBlocks() const;

private:
    size_t blockSize_;
    std::vector<uint8_t> pool_;
    std::vector<bool> used_;
    std::mutex mutex_;
};

