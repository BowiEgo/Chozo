#pragma once

#include <Core/Math/Matrix4.hpp>
#include <Core/Math/Vector3.hpp>

namespace CZ {

class Matrix3 {
public:
    Matrix3() { SetIdentity(); }
    Matrix3(float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21,
            float m22) {
        m_Data[0] = m00;
        m_Data[1] = m01;
        m_Data[2] = m02;
        m_Data[3] = m10;
        m_Data[4] = m11;
        m_Data[5] = m12;
        m_Data[6] = m20;
        m_Data[7] = m21;
        m_Data[8] = m22;
    }
    explicit Matrix3(const Matrix4& m4) {
        const float* d = m4.Data();
        for (int i = 0; i < 9; ++i)
            m_Data[i] = d[i];
    }
    explicit Matrix3(const float* data) {
        for (int i = 0; i < 9; ++i)
            m_Data[i] = data[i];
    }

    const float* Data() const { return m_Data; }
    float* Data() { return m_Data; }

    float& operator[](int index) { return m_Data[index]; }
    const float& operator[](int index) const { return m_Data[index]; }

    float& operator()(int row, int col) { return m_Data[col * 3 + row]; }
    float operator()(int row, int col) const { return m_Data[col * 3 + row]; }

    Matrix3 operator*(const Matrix3& other) const;
    Vector3 operator*(const Vector3& v) const;

    Matrix3 Transpose() const;
    float Determinant() const;
    Matrix3 Inverse() const;
    void SetIdentity();
    std::string ToString() const;

    static Matrix3 Identity();
    static Matrix3 RotationX(float angleDegrees);
    static Matrix3 RotationY(float angleDegrees);
    static Matrix3 RotationZ(float angleDegrees);
    static Matrix3 FromAxisAngle(const Vector3& axis, float angleDegrees);

private:
    float m_Data[9];
};

} // namespace CZ
