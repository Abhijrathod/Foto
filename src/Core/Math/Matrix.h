#pragma once
#include "Vector2D.h"
#include <array>

// 3x3 transformation matrix for 2D graphics
class Matrix {
public:
    Matrix();
    Matrix(float m00, float m01, float m02,
           float m10, float m11, float m12,
           float m20, float m21, float m22);

    static Matrix Identity();
    static Matrix Translation(float x, float y);
    static Matrix Translation(const Vector2D& v);
    static Matrix Rotation(float angle);
    static Matrix Scale(float sx, float sy);
    static Matrix Scale(float s);

    Matrix operator*(const Matrix& other) const;
    Vector2D TransformPoint(const Vector2D& point) const;
    Vector2D TransformVector(const Vector2D& vec) const;

    Matrix Inverse() const;
    float Determinant() const;

    float& operator()(int row, int col) { return data_[row * 3 + col]; }
    const float& operator()(int row, int col) const { return data_[row * 3 + col]; }

    const float* Data() const { return data_.data(); }
    float* Data() { return data_.data(); }

private:
    std::array<float, 9> data_;
};

