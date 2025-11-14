#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include <mutex>

// Manages image buffer memory (RGBA8 format)
class BufferManager {
public:
    struct Buffer {
        uint8_t* data = nullptr;
        uint32_t width = 0;
        uint32_t height = 0;
        size_t size = 0; // width * height * 4
    };

    static Buffer Create(uint32_t width, uint32_t height);
    static void Destroy(Buffer& buffer);
    static Buffer Clone(const Buffer& source);
    
    // Copy operations
    static void Copy(const Buffer& src, Buffer& dst);
    static void CopyRegion(const Buffer& src, int srcX, int srcY,
                          Buffer& dst, int dstX, int dstY,
                          uint32_t width, uint32_t height);

    // Clear buffer
    static void Clear(Buffer& buffer, uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 255);
    static void ClearRegion(Buffer& buffer, int x, int y, uint32_t w, uint32_t h,
                           uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 255);

    // Pixel access
    static uint8_t* GetPixel(Buffer& buffer, uint32_t x, uint32_t y);
    static const uint8_t* GetPixel(const Buffer& buffer, uint32_t x, uint32_t y);
};

