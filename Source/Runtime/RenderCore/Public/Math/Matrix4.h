// Matrix4.h
#pragma once

#include "Vector3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static_assert(sizeof(glm::mat4) == 64, "glm::mat4 should be 64 bytes");

class FMatrix4 {
public:
    // ===== Constructors =====
    FMatrix4() { SetIdentity(); }

    FMatrix4(const glm::mat4& m) { memcpy(m_Data, glm::value_ptr(m), 64); }

    // Construct from 16 floats
    FMatrix4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13,
             float m20, float m21, float m22, float m23, float m30, float m31, float m32,
             float m33) {
        m_Data[0] = m00;
        m_Data[1] = m01;
        m_Data[2] = m02;
        m_Data[3] = m03;
        m_Data[4] = m10;
        m_Data[5] = m11;
        m_Data[6] = m12;
        m_Data[7] = m13;
        m_Data[8] = m20;
        m_Data[9] = m21;
        m_Data[10] = m22;
        m_Data[11] = m23;
        m_Data[12] = m30;
        m_Data[13] = m31;
        m_Data[14] = m32;
        m_Data[15] = m33;
    }

    // ===== Conversion to GLM =====
    operator glm::mat4() const { return glm::make_mat4(m_Data); }

    // ===== Data access =====
    const float* Data() const { return m_Data; }
    float* Data() { return m_Data; }

    // ===== To String =====
    std::string ToString() const {
        return fmt::format("[{:6.2f} {:6.2f} {:6.2f} {:6.2f}]\n"
                           "[{:6.2f} {:6.2f} {:6.2f} {:6.2f}]\n"
                           "[{:6.2f} {:6.2f} {:6.2f} {:6.2f}]\n"
                           "[{:6.2f} {:6.2f} {:6.2f} {:6.2f}]",
                           m_Data[0], m_Data[1], m_Data[2], m_Data[3], m_Data[4], m_Data[5],
                           m_Data[6], m_Data[7], m_Data[8], m_Data[9], m_Data[10], m_Data[11],
                           m_Data[12], m_Data[13], m_Data[14], m_Data[15]);
    }

    std::string ToStringCompact() const {
        return fmt::format("[{:6.2f},{:6.2f},{:6.2f},{:6.2f};{:6.2f},{:6.2f},{:6.2f},{:6.2f};{:6."
                           "2f},{:6.2f},{:6.2f},{:6.2f};{:6.2f},{:6.2f},{:6.2f},{:6.2f}]",
                           m_Data[0], m_Data[1], m_Data[2], m_Data[3], m_Data[4], m_Data[5],
                           m_Data[6], m_Data[7], m_Data[8], m_Data[9], m_Data[10], m_Data[11],
                           m_Data[12], m_Data[13], m_Data[14], m_Data[15]);
    }

    // ===== Common matrices =====
    static FMatrix4 Identity() {
        FMatrix4 result;
        result.SetIdentity();
        return result;
    }

    static FMatrix4 Perspective(float fovDegrees, float aspect, float nearZ, float farZ) {
        glm::mat4 proj = glm::perspective(glm::radians(fovDegrees), aspect, nearZ, farZ);
        // Vulkan needs Y-axis flipped and depth range adjusted
        proj[1][1] *= -1;
        return FMatrix4(proj);
    }

    static FMatrix4 LookAt(const FVector3& eye, const FVector3& center, const FVector3& up) {
        return FMatrix4(glm::lookAt(eye.ToGLM(), center.ToGLM(), up.ToGLM()));
    }

    static FMatrix4 Translate(const FVector3& translation) {
        return FMatrix4(glm::translate(glm::mat4(1.0f), translation.ToGLM()));
    }

    static FMatrix4 Rotate(float angleDegrees, const FVector3& axis) {
        return FMatrix4(glm::rotate(glm::mat4(1.0f), glm::radians(angleDegrees), axis.ToGLM()));
    }

    static FMatrix4 Scale(const FVector3& scale) {
        return FMatrix4(glm::scale(glm::mat4(1.0f), scale.ToGLM()));
    }

    // ===== Operators =====
    FVector3 operator*(const FVector3& v) const {
        glm::vec4 result = glm::mat4(*this) * glm::vec4(v.x, v.y, v.z, 1.0f);
        return FVector3(result.x, result.y, result.z);
    }

    FMatrix4 operator*(const FMatrix4& other) const {
        return FMatrix4(glm::mat4(*this) * glm::mat4(other));
    }

    FMatrix4& operator*=(const FMatrix4& other) {
        *this = *this * other;
        return *this;
    }

private:
    void SetIdentity() {
        memset(m_Data, 0, 64);
        m_Data[0] = m_Data[5] = m_Data[10] = m_Data[15] = 1.0f;
    }

private:
    float m_Data[16];
};

inline std::ostream& operator<<(std::ostream& os, const FMatrix4& mat) {
    os << mat.ToString();
    return os;
}