#include "ColorSpace.h"
#include <algorithm>
#include <cmath>

HSLColor ColorSpace::RGBToHSL(const RGBColor& rgb) {
    float r = rgb.r;
    float g = rgb.g;
    float b = rgb.b;

    float max = std::max({r, g, b});
    float min = std::min({r, g, b});
    float delta = max - min;

    HSLColor hsl;
    hsl.l = (max + min) / 2.0f;
    hsl.a = rgb.a;

    if (delta < 0.0001f) {
        hsl.h = 0.0f;
        hsl.s = 0.0f;
        return hsl;
    }

    hsl.s = (hsl.l > 0.5f) ? delta / (2.0f - max - min) : delta / (max + min);

    if (max == r) {
        hsl.h = 60.0f * (((g - b) / delta) + (g < b ? 6.0f : 0.0f));
    } else if (max == g) {
        hsl.h = 60.0f * (((b - r) / delta) + 2.0f);
    } else {
        hsl.h = 60.0f * (((r - g) / delta) + 4.0f);
    }

    return hsl;
}

RGBColor ColorSpace::HSLToRGB(const HSLColor& hsl) {
    float h = hsl.h / 360.0f;
    float s = hsl.s;
    float l = hsl.l;

    RGBColor rgb;
    rgb.a = hsl.a;

    if (s < 0.0001f) {
        rgb.r = rgb.g = rgb.b = l;
        return rgb;
    }

    float q = (l < 0.5f) ? l * (1.0f + s) : l + s - l * s;
    float p = 2.0f * l - q;

    auto hue2rgb = [](float p, float q, float t) {
        if (t < 0.0f) t += 1.0f;
        if (t > 1.0f) t -= 1.0f;
        if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
        if (t < 1.0f / 2.0f) return q;
        if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
        return p;
    };

    rgb.r = hue2rgb(p, q, h + 1.0f / 3.0f);
    rgb.g = hue2rgb(p, q, h);
    rgb.b = hue2rgb(p, q, h - 1.0f / 3.0f);

    return rgb;
}

void ColorSpace::RGBToXYZ(const RGBColor& rgb, float& x, float& y, float& z) {
    float r = (rgb.r > 0.04045f) ? std::pow((rgb.r + 0.055f) / 1.055f, 2.4f) : rgb.r / 12.92f;
    float g = (rgb.g > 0.04045f) ? std::pow((rgb.g + 0.055f) / 1.055f, 2.4f) : rgb.g / 12.92f;
    float b = (rgb.b > 0.04045f) ? std::pow((rgb.b + 0.055f) / 1.055f, 2.4f) : rgb.b / 12.92f;

    x = r * 0.4124564f + g * 0.3575761f + b * 0.1804375f;
    y = r * 0.2126729f + g * 0.7151522f + b * 0.0721750f;
    z = r * 0.0193339f + g * 0.1191920f + b * 0.9503041f;
}

RGBColor ColorSpace::XYZToRGB(float x, float y, float z) {
    float r = x * 3.2404542f - y * 1.5371385f - z * 0.4985314f;
    float g = -x * 0.9692660f + y * 1.8760108f + z * 0.0415560f;
    float b = x * 0.0556434f - y * 0.2040259f + z * 1.0572252f;

    r = (r > 0.0031308f) ? 1.055f * std::pow(r, 1.0f / 2.4f) - 0.055f : 12.92f * r;
    g = (g > 0.0031308f) ? 1.055f * std::pow(g, 1.0f / 2.4f) - 0.055f : 12.92f * g;
    b = (b > 0.0031308f) ? 1.055f * std::pow(b, 1.0f / 2.4f) - 0.055f : 12.92f * b;

    return RGBColor(
        std::max(0.0f, std::min(1.0f, r)),
        std::max(0.0f, std::min(1.0f, g)),
        std::max(0.0f, std::min(1.0f, b)),
        1.0f
    );
}

LABColor ColorSpace::RGBToLAB(const RGBColor& rgb) {
    float x, y, z;
    RGBToXYZ(rgb, x, y, z);

    // D65 white point
    x /= 0.95047f;
    z /= 1.08883f;

    auto f = [](float t) {
        const float delta = 6.0f / 29.0f;
        return (t > delta * delta * delta) ? std::cbrt(t) : t / (3.0f * delta * delta) + 4.0f / 29.0f;
    };

    float fx = f(x);
    float fy = f(y);
    float fz = f(z);

    LABColor lab;
    lab.l = 116.0f * fy - 16.0f;
    lab.a = 500.0f * (fx - fy);
    lab.b = 200.0f * (fy - fz);
    lab.alpha = rgb.a;

    return lab;
}

RGBColor ColorSpace::LABToRGB(const LABColor& lab) {
    float y = (lab.l + 16.0f) / 116.0f;
    float x = lab.a / 500.0f + y;
    float z = y - lab.b / 200.0f;

    auto f_inv = [](float t) {
        const float delta = 6.0f / 29.0f;
        return (t > delta) ? t * t * t : 3.0f * delta * delta * (t - 4.0f / 29.0f);
    };

    x = f_inv(x) * 0.95047f;
    y = f_inv(y);
    z = f_inv(z) * 1.08883f;

    RGBColor rgb = XYZToRGB(x, y, z);
    rgb.a = lab.alpha;
    return rgb;
}

RGBColor ColorSpace::Blend(const RGBColor& a, const RGBColor& b, float t) {
    t = std::max(0.0f, std::min(1.0f, t));
    return RGBColor(
        a.r + (b.r - a.r) * t,
        a.g + (b.g - a.g) * t,
        a.b + (b.b - a.b) * t,
        a.a + (b.a - a.a) * t
    );
}

RGBColor ColorSpace::PremultiplyAlpha(const RGBColor& c) {
    return RGBColor(c.r * c.a, c.g * c.a, c.b * c.a, c.a);
}

RGBColor ColorSpace::UnpremultiplyAlpha(const RGBColor& c) {
    if (c.a < 0.0001f) return RGBColor(0.0f, 0.0f, 0.0f, 0.0f);
    return RGBColor(c.r / c.a, c.g / c.a, c.b / c.a, c.a);
}

float ColorSpace::LinearToSRGB(float linear) {
    return (linear <= 0.0031308f) ? linear * 12.92f : 1.055f * std::pow(linear, 1.0f / 2.4f) - 0.055f;
}

float ColorSpace::SRGBToLinear(float srgb) {
    return (srgb <= 0.04045f) ? srgb / 12.92f : std::pow((srgb + 0.055f) / 1.055f, 2.4f);
}

RGBColor ColorSpace::LinearToSRGB(const RGBColor& linear) {
    return RGBColor(
        LinearToSRGB(linear.r),
        LinearToSRGB(linear.g),
        LinearToSRGB(linear.b),
        linear.a
    );
}

RGBColor ColorSpace::SRGBToLinear(const RGBColor& srgb) {
    return RGBColor(
        SRGBToLinear(srgb.r),
        SRGBToLinear(srgb.g),
        SRGBToLinear(srgb.b),
        srgb.a
    );
}

