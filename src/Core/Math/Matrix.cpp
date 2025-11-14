#include "Matrix.h"
#include <cstring>

Matrix::Matrix() {
    *this = Identity();
}

Matrix::Matrix(float m00, float m01, float m02,
               float m10, float m11, float m12,
               float m20, float m21, float m22) {
    data_[0] = m00; data_[1] = m01; data_[2] = m02;
    data_[3] = m10; data_[4] = m11; data_[5] = m12;
    data_[6] = m20; data_[7] = m21; data_[8] = m22;
}

Matrix Matrix::Identity() {
    Matrix m;
    m.data_[0] = 1.0f; m.data_[1] = 0.0f; m.data_[2] = 0.0f;
    m.data_[3] = 0.0f; m.data_[4] = 1.0f; m.data_[5] = 0.0f;
    m.data_[6] = 0.0f; m.data_[7] = 0.0f; m.data_[8] = 1.0f;
    return m;
}

Matrix Matrix::Translation(float x, float y) {
    Matrix m = Identity();
    m(0, 2) = x;
    m(1, 2) = y;
    return m;
}

Matrix Matrix::Translation(const Vector2D& v) {
    return Translation(v.x, v.y);
}

Matrix Matrix::Rotation(float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);
    Matrix m = Identity();
    m(0, 0) = c; m(0, 1) = -s;
    m(1, 0) = s; m(1, 1) = c;
    return m;
}

Matrix Matrix::Scale(float sx, float sy) {
    Matrix m = Identity();
    m(0, 0) = sx;
    m(1, 1) = sy;
    return m;
}

Matrix Matrix::Scale(float s) {
    return Scale(s, s);
}

Matrix Matrix::operator*(const Matrix& other) const {
    Matrix result;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            float sum = 0.0f;
            for (int k = 0; k < 3; k++) {
                sum += (*this)(i, k) * other(k, j);
            }
            result(i, j) = sum;
        }
    }
    return result;
}

Vector2D Matrix::TransformPoint(const Vector2D& point) const {
    float x = (*this)(0, 0) * point.x + (*this)(0, 1) * point.y + (*this)(0, 2);
    float y = (*this)(1, 0) * point.x + (*this)(1, 1) * point.y + (*this)(1, 2);
    return Vector2D(x, y);
}

Vector2D Matrix::TransformVector(const Vector2D& vec) const {
    float x = (*this)(0, 0) * vec.x + (*this)(0, 1) * vec.y;
    float y = (*this)(1, 0) * vec.x + (*this)(1, 1) * vec.y;
    return Vector2D(x, y);
}

float Matrix::Determinant() const {
    return (*this)(0, 0) * ((*this)(1, 1) * (*this)(2, 2) - (*this)(1, 2) * (*this)(2, 1))
         - (*this)(0, 1) * ((*this)(1, 0) * (*this)(2, 2) - (*this)(1, 2) * (*this)(2, 0))
         + (*this)(0, 2) * ((*this)(1, 0) * (*this)(2, 1) - (*this)(1, 1) * (*this)(2, 0));
}

Matrix Matrix::Inverse() const {
    float det = Determinant();
    if (std::abs(det) < 1e-6f) return Identity(); // Singular matrix

    float invDet = 1.0f / det;
    Matrix result;
    
    result(0, 0) = ((*this)(1, 1) * (*this)(2, 2) - (*this)(1, 2) * (*this)(2, 1)) * invDet;
    result(0, 1) = ((*this)(0, 2) * (*this)(2, 1) - (*this)(0, 1) * (*this)(2, 2)) * invDet;
    result(0, 2) = ((*this)(0, 1) * (*this)(1, 2) - (*this)(0, 2) * (*this)(1, 1)) * invDet;
    result(1, 0) = ((*this)(1, 2) * (*this)(2, 0) - (*this)(1, 0) * (*this)(2, 2)) * invDet;
    result(1, 1) = ((*this)(0, 0) * (*this)(2, 2) - (*this)(0, 2) * (*this)(2, 0)) * invDet;
    result(1, 2) = ((*this)(0, 2) * (*this)(1, 0) - (*this)(0, 0) * (*this)(1, 2)) * invDet;
    result(2, 0) = ((*this)(1, 0) * (*this)(2, 1) - (*this)(1, 1) * (*this)(2, 0)) * invDet;
    result(2, 1) = ((*this)(0, 1) * (*this)(2, 0) - (*this)(0, 0) * (*this)(2, 1)) * invDet;
    result(2, 2) = ((*this)(0, 0) * (*this)(1, 1) - (*this)(0, 1) * (*this)(1, 0)) * invDet;
    
    return result;
}

