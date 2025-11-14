#pragma once
#include "../Math/ColorSpace.h"

class ColorEngine {
public:
    ColorEngine();
    ~ColorEngine();

    // Color conversion utilities
    RGBColor ConvertToRGB(const HSLColor& hsl) const;
    HSLColor ConvertToHSL(const RGBColor& rgb) const;
    
    // Color adjustments
    RGBColor AdjustBrightness(const RGBColor& color, float amount) const;
    RGBColor AdjustContrast(const RGBColor& color, float amount) const;
    RGBColor AdjustSaturation(const RGBColor& color, float amount) const;
    RGBColor AdjustHue(const RGBColor& color, float amount) const;
};

