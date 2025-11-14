#pragma once
#include <cstdint>

struct RGBColor {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;

    RGBColor() = default;
    RGBColor(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
    RGBColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
        : r(r / 255.0f), g(g / 255.0f), b(b / 255.0f), a(a / 255.0f) {}
};

struct HSLColor {
    float h = 0.0f; // 0-360
    float s = 0.0f; // 0-1
    float l = 0.0f; // 0-1
    float a = 1.0f;
};

struct LABColor {
    float l = 0.0f; // 0-100
    float a = 0.0f; // -128 to 127
    float b = 0.0f; // -128 to 127
    float alpha = 1.0f;
};

class ColorSpace {
public:
    // RGB <-> HSL conversions
    static HSLColor RGBToHSL(const RGBColor& rgb);
    static RGBColor HSLToRGB(const HSLColor& hsl);

    // RGB <-> LAB conversions (via XYZ)
    static LABColor RGBToLAB(const RGBColor& rgb);
    static RGBColor LABToRGB(const LABColor& lab);

    // RGB <-> XYZ conversions
    static void RGBToXYZ(const RGBColor& rgb, float& x, float& y, float& z);
    static RGBColor XYZToRGB(float x, float y, float z);

    // Color manipulation
    static RGBColor Blend(const RGBColor& a, const RGBColor& b, float t);
    static RGBColor PremultiplyAlpha(const RGBColor& c);
    static RGBColor UnpremultiplyAlpha(const RGBColor& c);

    // Gamma correction
    static float LinearToSRGB(float linear);
    static float SRGBToLinear(float srgb);
    static RGBColor LinearToSRGB(const RGBColor& linear);
    static RGBColor SRGBToLinear(const RGBColor& srgb);
};

