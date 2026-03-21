// Matrix3.h
#pragma once

#include "Matrix4.h"
#include "Vector3.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/fmt/bundled/format.h>

static_assert(sizeof(glm::mat3) == 36, "glm::mat3 should be 36 bytes");
static_assert(alignof(glm::mat3) == 4, "glm::mat3 should be 4-byte aligned");

class FMatrix3 {
public:
    FMatrix3() { SetIdentity(); }

    FMatrix3(const glm::mat3& m) { memcpy(m_Data, glm::value_ptr(m), 36); }

    FMatrix3(float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21,
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

    explicit FMatrix3(const FMatrix4& m4) {
        const float* d = m4.Data();
        m_Data[0] = d[0];
        m_Data[1] = d[1];
        m_Data[2] = d[2];
        m_Data[3] = d[4];
        m_Data[4] = d[5];
        m_Data[5] = d[6];
        m_Data[6] = d[8];
        m_Data[7] = d[9];
        m_Data[8] = d[10];
    }

    operator glm::mat3() const { return glm::make_mat3(m_Data); }

    const float* Data() const { return m_Data; }
    float* Data() { return m_Data; }

    float& operator[](int index) { return m_Data[index]; }
    const float& operator[](int index) const { return m_Data[index]; }

    float& operator()(int row, int col) { return m_Data[col * 3 + row]; }
    float operator()(int row, int col) const { return m_Data[col * 3 + row]; }

    FMatrix3 operator*(const FMatrix3& other) const {
        return FMatrix3(glm::mat3(*this) * glm::mat3(other));
    }

    FVector3 operator*(const FVector3& v) const {
        glm::vec3 result = glm::mat3(*this) * glm::vec3(v.x, v.y, v.z);
        return FVector3(result.x, result.y, result.z);
    }

    FMatrix3 Transpose() const {
        glm::mat3 glmMat = *this;
        return FMatrix3(glm::transpose(glmMat));
    }

    float Determinant() const { return glm::determinant(glm::mat3(*this)); }

    FMatrix3 Inverse() const { return FMatrix3(glm::inverse(glm::mat3(*this))); }

    void SetIdentity() {
        memset(m_Data, 0, 36);
        m_Data[0] = m_Data[4] = m_Data[8] = 1.0f;
    }

    std::string ToString() const {
        return fmt::format("[{:6.3f} {:6.3f} {:6.3f}]\n"
                           "[{:6.3f} {:6.3f} {:6.3f}]\n"
                           "[{:6.3f} {:6.3f} {:6.3f}]",
                           m_Data[0], m_Data[1], m_Data[2], m_Data[3], m_Data[4], m_Data[5],
                           m_Data[6], m_Data[7], m_Data[8]);
    }

    static FMatrix3 Identity() {
        FMatrix3 result;
        result.SetIdentity();
        return result;
    }

    static FMatrix3 RotationX(float angleDegrees) {
        float rad = glm::radians(angleDegrees);
        float c = cosf(rad);
        float s = sinf(rad);
        return FMatrix3(1, 0, 0, 0, c, -s, 0, s, c);
    }

    static FMatrix3 RotationY(float angleDegrees) {
        float rad = glm::radians(angleDegrees);
        float c = cosf(rad);
        float s = sinf(rad);
        return FMatrix3(c, 0, s, 0, 1, 0, -s, 0, c);
    }

    static FMatrix3 RotationZ(float angleDegrees) {
        float rad = glm::radians(angleDegrees);
        float c = cosf(rad);
        float s = sinf(rad);
        return FMatrix3(c, -s, 0, s, c, 0, 0, 0, 1);
    }

    static FMatrix3 FromAxisAngle(const FVector3& axis, float angleDegrees) {
        float rad = glm::radians(angleDegrees);
        glm::mat3 glmMat = glm::mat3_cast(glm::angleAxis(rad, axis.ToGLM()));
        return FMatrix3(glmMat);
    }

private:
    float m_Data[9];
};

inline FMatrix3 operator*(float s, const FMatrix3& m) {
    glm::mat3 glmMat = m;
    return FMatrix3(s * glmMat);
}