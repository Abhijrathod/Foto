#include "BufferManager.h"
#include <cstring>
#include <algorithm>

BufferManager::Buffer BufferManager::Create(uint32_t width, uint32_t height) {
    Buffer buffer;
    buffer.width = width;
    buffer.height = height;
    buffer.size = static_cast<size_t>(width) * height * 4;
    buffer.data = new uint8_t[buffer.size];
    return buffer;
}

void BufferManager::Destroy(Buffer& buffer) {
    if (buffer.data) {
        delete[] buffer.data;
        buffer.data = nullptr;
        buffer.width = 0;
        buffer.height = 0;
        buffer.size = 0;
    }
}

BufferManager::Buffer BufferManager::Clone(const Buffer& source) {
    Buffer buffer = Create(source.width, source.height);
    if (source.data && buffer.data) {
        std::memcpy(buffer.data, source.data, buffer.size);
    }
    return buffer;
}

void BufferManager::Copy(const Buffer& src, Buffer& dst) {
    if (src.width != dst.width || src.height != dst.height) {
        return; // Size mismatch
    }
    if (src.data && dst.data) {
        std::memcpy(dst.data, src.data, dst.size);
    }
}

void BufferManager::CopyRegion(const Buffer& src, int srcX, int srcY,
                               Buffer& dst, int dstX, int dstY,
                               uint32_t width, uint32_t height) {
    if (!src.data || !dst.data) return;
    
    // Clamp to valid regions
    int srcEndX = std::min(static_cast<int>(srcX + width), static_cast<int>(src.width));
    int srcEndY = std::min(static_cast<int>(srcY + height), static_cast<int>(src.height));
    int dstEndX = std::min(static_cast<int>(dstX + width), static_cast<int>(dst.width));
    int dstEndY = std::min(static_cast<int>(dstY + height), static_cast<int>(dst.height));
    
    int copyWidth = std::min(srcEndX - srcX, dstEndX - dstX);
    int copyHeight = std::min(srcEndY - srcY, dstEndY - dstY);
    
    if (copyWidth <= 0 || copyHeight <= 0) return;
    
    for (int y = 0; y < copyHeight; y++) {
        const uint8_t* srcRow = GetPixel(src, srcX, srcY + y);
        uint8_t* dstRow = GetPixel(dst, dstX, dstY + y);
        std::memcpy(dstRow, srcRow, copyWidth * 4);
    }
}

void BufferManager::Clear(Buffer& buffer, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (!buffer.data) return;
    
    for (size_t i = 0; i < buffer.size; i += 4) {
        buffer.data[i + 0] = r;
        buffer.data[i + 1] = g;
        buffer.data[i + 2] = b;
        buffer.data[i + 3] = a;
    }
}

void BufferManager::ClearRegion(Buffer& buffer, int x, int y, uint32_t w, uint32_t h,
                                uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (!buffer.data) return;
    
    int endX = std::min(static_cast<int>(x + w), static_cast<int>(buffer.width));
    int endY = std::min(static_cast<int>(y + h), static_cast<int>(buffer.height));
    
    for (int py = std::max(0, y); py < endY; py++) {
        for (int px = std::max(0, x); px < endX; px++) {
            uint8_t* pixel = GetPixel(buffer, px, py);
            pixel[0] = r;
            pixel[1] = g;
            pixel[2] = b;
            pixel[3] = a;
        }
    }
}

uint8_t* BufferManager::GetPixel(Buffer& buffer, uint32_t x, uint32_t y) {
    if (x >= buffer.width || y >= buffer.height || !buffer.data) {
        return nullptr;
    }
    return buffer.data + (y * buffer.width + x) * 4;
}

const uint8_t* BufferManager::GetPixel(const Buffer& buffer, uint32_t x, uint32_t y) {
    if (x >= buffer.width || y >= buffer.height || !buffer.data) {
        return nullptr;
    }
    return buffer.data + (y * buffer.width + x) * 4;
}

