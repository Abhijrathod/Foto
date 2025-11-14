#include "ColorEngine.h"
#include <cmath>
#include <algorithm>

ColorEngine::ColorEngine() {
}

ColorEngine::~ColorEngine() {
}

RGBColor ColorEngine::ConvertToRGB(const HSLColor& hsl) const {
    return ColorSpace::HSLToRGB(hsl);
}

HSLColor ColorEngine::ConvertToHSL(const RGBColor& rgb) const {
    return ColorSpace::RGBToHSL(rgb);
}

RGBColor ColorEngine::AdjustBrightness(const RGBColor& color, float amount) const {
    float factor = 1.0f + amount;
    return RGBColor(
        std::max(0.0f, std::min(1.0f, color.r * factor)),
        std::max(0.0f, std::min(1.0f, color.g * factor)),
        std::max(0.0f, std::min(1.0f, color.b * factor)),
        color.a
    );
}

RGBColor ColorEngine::AdjustContrast(const RGBColor& color, float amount) const {
    float factor = (1.0f + amount) / (1.0f - amount);
    float midpoint = 0.5f;
    return RGBColor(
        std::max(0.0f, std::min(1.0f, (color.r - midpoint) * factor + midpoint)),
        std::max(0.0f, std::min(1.0f, (color.g - midpoint) * factor + midpoint)),
        std::max(0.0f, std::min(1.0f, (color.b - midpoint) * factor + midpoint)),
        color.a
    );
}

RGBColor ColorEngine::AdjustSaturation(const RGBColor& color, float amount) const {
    HSLColor hsl = ConvertToHSL(color);
    hsl.s = std::max(0.0f, std::min(1.0f, hsl.s * (1.0f + amount)));
    return ConvertToRGB(hsl);
}

RGBColor ColorEngine::AdjustHue(const RGBColor& color, float amount) const {
    HSLColor hsl = ConvertToHSL(color);
    hsl.h = std::fmod(hsl.h + amount * 360.0f, 360.0f);
    if (hsl.h < 0.0f) hsl.h += 360.0f;
    return ConvertToRGB(hsl);
}

