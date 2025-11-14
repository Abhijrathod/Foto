#pragma once
#include <cmath>
#include <cstdint>

struct Vector2D {
    float x = 0.0f;
    float y = 0.0f;

    Vector2D() = default;
    Vector2D(float x, float y) : x(x), y(y) {}
    Vector2D(int x, int y) : x((float)x), y((float)y) {}

    Vector2D operator+(const Vector2D& other) const { return Vector2D(x + other.x, y + other.y); }
    Vector2D operator-(const Vector2D& other) const { return Vector2D(x - other.x, y - other.y); }
    Vector2D operator*(float scalar) const { return Vector2D(x * scalar, y * scalar); }
    Vector2D operator/(float scalar) const { return Vector2D(x / scalar, y / scalar); }

    Vector2D& operator+=(const Vector2D& other) { x += other.x; y += other.y; return *this; }
    Vector2D& operator-=(const Vector2D& other) { x -= other.x; y -= other.y; return *this; }
    Vector2D& operator*=(float scalar) { x *= scalar; y *= scalar; return *this; }

    float Length() const { return std::sqrt(x * x + y * y); }
    float LengthSquared() const { return x * x + y * y; }
    Vector2D Normalized() const { float len = Length(); return len > 0.0f ? *this / len : Vector2D(); }
    
    float Dot(const Vector2D& other) const { return x * other.x + y * other.y; }
    float Cross(const Vector2D& other) const { return x * other.y - y * other.x; }

    Vector2D Rotate(float angle) const {
        float c = std::cos(angle);
        float s = std::sin(angle);
        return Vector2D(x * c - y * s, x * s + y * c);
    }
};

