#pragma once
#define NOMINMAX
#include <windows.h>
#ifdef LoadImage
#undef LoadImage
#endif
#include <wincodec.h>
#include <vector>
#include <cstdint>

struct ImageData {
    std::vector<uint8_t> pixels; // RGBA8
    UINT width = 0;
    UINT height = 0;
    bool valid = false;
};

class ImageCodecs {
public:
    static bool LoadImage(const wchar_t* filepath, ImageData& out);
    static bool SaveImage(const wchar_t* filepath, const ImageData& img);
};
