#include "TileCache.h"
#include <algorithm>

TileCache::TileCache(size_t maxTiles) : maxTiles_(maxTiles) {
}

TileCache::~TileCache() {
    Clear();
}

TileCache::Tile* TileCache::GetTile(uint32_t layerId, uint32_t tileX, uint32_t tileY) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    TileKey key{tileX, tileY, layerId};
    auto it = tiles_.find(key);
    if (it != tiles_.end()) {
        it->second.lastAccess = ++accessCounter_;
        return &it->second;
    }
    return nullptr;
}

TileCache::Tile* TileCache::GetOrCreateTile(uint32_t layerId, uint32_t tileX, uint32_t tileY) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    TileKey key{tileX, tileY, layerId};
    auto it = tiles_.find(key);
    
    if (it != tiles_.end()) {
        it->second.lastAccess = ++accessCounter_;
        return &it->second;
    }
    
    // Create new tile
    if (tiles_.size() >= maxTiles_) {
        EvictLRU();
    }
    
    Tile tile;
    tile.buffer = BufferManager::Create(TILE_SIZE, TILE_SIZE);
    tile.lastAccess = ++accessCounter_;
    
    auto result = tiles_.emplace(key, std::move(tile));
    return &result.first->second;
}

void TileCache::MarkDirty(uint32_t layerId, uint32_t tileX, uint32_t tileY) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    TileKey key{tileX, tileY, layerId};
    auto it = tiles_.find(key);
    if (it != tiles_.end()) {
        it->second.dirty = true;
    }
}

void TileCache::ClearLayer(uint32_t layerId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = tiles_.begin();
    while (it != tiles_.end()) {
        if (it->first.layerId == layerId) {
            BufferManager::Destroy(it->second.buffer);
            it = tiles_.erase(it);
        } else {
            ++it;
        }
    }
}

void TileCache::Clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& pair : tiles_) {
        BufferManager::Destroy(pair.second.buffer);
    }
    tiles_.clear();
}

size_t TileCache::GetTileCount() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex_));
    return tiles_.size();
}

void TileCache::EvictLRU() {
    if (tiles_.empty()) return;
    
    auto oldest = tiles_.begin();
    for (auto it = tiles_.begin(); it != tiles_.end(); ++it) {
        if (it->second.lastAccess < oldest->second.lastAccess) {
            oldest = it;
        }
    }
    
    BufferManager::Destroy(oldest->second.buffer);
    tiles_.erase(oldest);
}

