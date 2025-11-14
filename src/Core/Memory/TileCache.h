#pragma once
#include "BufferManager.h"
#include <cstdint>
#include <unordered_map>
#include <mutex>
#include <string>

// Tile-based caching for large images
class TileCache {
public:
    static constexpr uint32_t TILE_SIZE = 256;

    struct TileKey {
        uint32_t tileX;
        uint32_t tileY;
        uint32_t layerId;
        
        bool operator==(const TileKey& other) const {
            return tileX == other.tileX && tileY == other.tileY && layerId == other.layerId;
        }
    };

    struct Tile {
        BufferManager::Buffer buffer;
        bool dirty = false;
        uint64_t lastAccess = 0;
    };

    TileCache(size_t maxTiles = 1024);
    ~TileCache();

    Tile* GetTile(uint32_t layerId, uint32_t tileX, uint32_t tileY);
    Tile* GetOrCreateTile(uint32_t layerId, uint32_t tileX, uint32_t tileY);
    
    void MarkDirty(uint32_t layerId, uint32_t tileX, uint32_t tileY);
    void ClearLayer(uint32_t layerId);
    void Clear();

    size_t GetTileCount() const;
    size_t GetMaxTiles() const { return maxTiles_; }

private:
    struct TileKeyHash {
        size_t operator()(const TileKey& key) const {
            return (static_cast<size_t>(key.layerId) << 32) | 
                   (static_cast<size_t>(key.tileX) << 16) | 
                   static_cast<size_t>(key.tileY);
        }
    };

    void EvictLRU();

    std::unordered_map<TileKey, Tile, TileKeyHash> tiles_;
    size_t maxTiles_;
    mutable std::mutex mutex_;
    uint64_t accessCounter_ = 0;
};

