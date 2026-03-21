#pragma once

#include "RendererAPI.h"
#include "Vector3.h"
#include "Vector4.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/fmt/bundled/format.h>

static_assert(sizeof(glm::mat4) == 64, "glm::mat4 should be 64 bytes");

class FQuaternion;

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
        switch (FRendererAPI::GetType()) {
            case FRendererAPI::EType::Vulkan:
                return VulkanPerspective(fovDegrees, aspect, nearZ, farZ);

            case FRendererAPI::EType::OpenGL:
                return OpenGLPerspective(fovDegrees, aspect, nearZ, farZ);

            default: return VulkanPerspective(fovDegrees, aspect, nearZ, farZ);
        }
    }

    static FMatrix4 Orthographic(float left, float right, float bottom, float top, float nearZ,
                                 float farZ) {
        // clang-format off
        float projData[16] = {
            2.0f / (right - left), 0.0f, 0.0f, 0.0f,
            0.0f, -2.0f / (top - bottom), 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f / (farZ - nearZ), 0.0f,
            -(right + left) / (right - left), -(top + bottom) / (top - bottom), -nearZ / (farZ - nearZ), 1.0f
        };
        return FMatrix4(projData[0], projData[1], projData[2], projData[3],
                        projData[4], projData[5], projData[6], projData[7],
                        projData[8], projData[9], projData[10], projData[11],
                        projData[12], projData[13], projData[14], projData[15]);
        // clang-format on
    }

    static FMatrix4 LookAt(const FVector3& eye, const FVector3& center, const FVector3& up) {
        return FMatrix4(glm::lookAt(eye.ToGLM(), center.ToGLM(), up.ToGLM()));
    }

    // ===== Transformations =====
    static FMatrix4 Translate(const FVector3& translation) {
        return FMatrix4(glm::translate(glm::mat4(1.0f), translation.ToGLM()));
    }

    static FMatrix4 Translate(float x, float y, float z) { return Translate(FVector3(x, y, z)); }

    static FMatrix4 Rotate(float angleDegrees, const FVector3& axis) {
        return FMatrix4(glm::rotate(glm::mat4(1.0f), glm::radians(angleDegrees), axis.ToGLM()));
    }

    static FMatrix4 Rotate(float angleDegrees, float axisX, float axisY, float axisZ) {
        return Rotate(angleDegrees, FVector3(axisX, axisY, axisZ));
    }

    static FMatrix4 Scale(const FVector3& scale) {
        return FMatrix4(glm::scale(glm::mat4(1.0f), scale.ToGLM()));
    }

    static FMatrix4 Scale(float x, float y, float z) { return Scale(FVector3(x, y, z)); }

    static FMatrix4 Scale(float uniform) { return Scale(FVector3(uniform, uniform, uniform)); }

    // ===== Matrix operations =====
    FMatrix4 Inverse() { return FMatrix4(glm::inverse(glm::mat4(*this))); }

    FMatrix4 Transpose() { return FMatrix4(glm::transpose(glm::mat4(*this))); }

    // ===== Decomposition (useful for extracting translation/rotation/scale) =====
    FVector3 GetTranslation() const { return FVector3(m_Data[12], m_Data[13], m_Data[14]); }

    FQuaternion GetRotation() const;

    FVector3 GetScale() const {
        glm::mat4 glmMat = *this;
        // Extract scale from matrix columns
        float scaleX = glm::length(glm::vec3(glmMat[0]));
        float scaleY = glm::length(glm::vec3(glmMat[1]));
        float scaleZ = glm::length(glm::vec3(glmMat[2]));
        return FVector3(scaleX, scaleY, scaleZ);
    }

    // ===== Utility =====
    bool IsIdentity(float tolerance = 1e-6f) const {
        FMatrix4 identity;
        for (int i = 0; i < 16; ++i) {
            if (std::abs(m_Data[i] - identity.m_Data[i]) > tolerance) {
                return false;
            }
        }
        return true;
    }

    float Determinant() const { return glm::determinant(glm::mat4(*this)); }

    // ===== Common transformation matrices =====
    static FMatrix4 Translation(float x, float y, float z) { return Translate(x, y, z); }

    static FMatrix4 RotationX(float angleDegrees) { return Rotate(angleDegrees, FVector3::Right); }

    static FMatrix4 RotationY(float angleDegrees) { return Rotate(angleDegrees, FVector3::Up); }

    static FMatrix4 RotationZ(float angleDegrees) {
        return Rotate(angleDegrees, FVector3::Forward);
    }

    static FMatrix4 TRS(const FVector3& translation, const FQuaternion& rotation,
                        const FVector3& scale);

    // ===== Operators =====
    FVector3 operator*(const FVector3& v) const {
        glm::vec4 result = glm::mat4(*this) * glm::vec4(v.x, v.y, v.z, 1.0f);
        return FVector3(result.x, result.y, result.z);
    }

    FVector4 operator*(const FVector4& v) const {
        return FVector4(m_Data[0] * v.x + m_Data[1] * v.y + m_Data[2] * v.z + m_Data[3] * v.w,
                        m_Data[4] * v.x + m_Data[5] * v.y + m_Data[6] * v.z + m_Data[7] * v.w,
                        m_Data[8] * v.x + m_Data[9] * v.y + m_Data[10] * v.z + m_Data[11] * v.w,
                        m_Data[12] * v.x + m_Data[13] * v.y + m_Data[14] * v.z + m_Data[15] * v.w);
    }

    FMatrix4 operator*(const FMatrix4& other) const {
        return FMatrix4(glm::mat4(*this) * glm::mat4(other));
    }

    FMatrix4& operator*=(const FMatrix4& other) {
        *this = *this * other;
        return *this;
    }

    float* operator[](int col) { return &m_Data[col * 4]; }

    const float* operator[](int col) const { return &m_Data[col * 4]; }

private:
    void SetIdentity() {
        memset(m_Data, 0, 64);
        m_Data[0] = m_Data[5] = m_Data[10] = m_Data[15] = 1.0f;
    }

    static FMatrix4 VulkanPerspective(float fov, float aspect, float nearZ, float farZ) {
        float tanHalfFov = tanf(glm::radians(fov) * 0.5f);
        return FMatrix4(1.0f / (aspect * tanHalfFov), 0, 0, 0, 0, -1.0f / tanHalfFov, 0, 0, 0, 0,
                        farZ / (farZ - nearZ), 1, 0, 0, -(nearZ * farZ) / (farZ - nearZ), 0);
    }

    static FMatrix4 OpenGLPerspective(float fov, float aspect, float nearZ, float farZ) {
        return glm::perspective(glm::radians(fov), aspect, nearZ, farZ);
    }

private:
    float m_Data[16];
};

inline std::ostream& operator<<(std::ostream& os, const FMatrix4& mat) {
    os << mat.ToString();
    return os;
}

// ===== String formatting for logs =====
// CZ_LOG(LogTemp, Info, "Matrix: {}", mat);     // Defaut
// CZ_LOG(LogTemp, Info, "Matrix: {:c}", mat);   // Compact
// CZ_LOG(LogTemp, Info, "Matrix: {:4}", mat);   // Specific precision
template <> struct fmt::formatter<FMatrix4> {
    // Optional format specifiers
    int precision = 2;
    bool compact = false;

    constexpr auto parse(format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it == ':') {
            ++it;
            // Simple format parsing: e.g., {:4.2c}
            if (*it == 'c') {
                compact = true;
                ++it;
            }
            // Parse precision
            if (*it != '}') {
                char* end;
                precision = std::strtol(it, &end, 10);
                it = end;
            }
        }
        if (it != ctx.end() && *it != '}') {
            throw format_error("invalid format");
        }
        return it;
    }

    template <typename FormatContext> auto format(const FMatrix4& mat, FormatContext& ctx) const {
        if (compact) {
            return fmt::format_to(ctx.out(), "{}", mat.ToStringCompact());
        }
        return fmt::format_to(ctx.out(), "{}", mat.ToString());
    }
};