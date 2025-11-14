#include "MemoryPool.h"
#include <algorithm>

MemoryPool::MemoryPool(size_t blockSize, size_t numBlocks)
    : blockSize_(blockSize), pool_(blockSize * numBlocks), used_(numBlocks, false) {
}

MemoryPool::~MemoryPool() {
}

void* MemoryPool::Allocate() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (size_t i = 0; i < used_.size(); i++) {
        if (!used_[i]) {
            used_[i] = true;
            return pool_.data() + i * blockSize_;
        }
    }
    
    return nullptr; // Pool exhausted
}

void MemoryPool::Deallocate(void* ptr) {
    if (!ptr) return;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    uint8_t* bytePtr = static_cast<uint8_t*>(ptr);
    if (bytePtr >= pool_.data() && bytePtr < pool_.data() + pool_.size()) {
        size_t index = (bytePtr - pool_.data()) / blockSize_;
        if (index < used_.size()) {
            used_[index] = false;
        }
    }
}

size_t MemoryPool::GetAvailableBlocks() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex_));
    return std::count(used_.begin(), used_.end(), false);
}

