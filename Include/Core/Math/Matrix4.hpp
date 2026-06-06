#pragma once

#include <Core/Header/RendererAPI.hpp>
#include <Core/Math/Vector3.hpp>
#include <Core/Math/Vector4.hpp>

#include <fmt/format.h>

namespace CZ {

class Quaternion;
class Matrix3;

class Matrix4 {
public:
    // ===== Constructors =====
    Matrix4() { SetIdentity(); }

    // Construct from 16 floats
    Matrix4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13,
            float m20, float m21, float m22, float m23, float m30, float m31, float m32,
            float m33) {
        m_Data[0]  = m00;
        m_Data[1]  = m01;
        m_Data[2]  = m02;
        m_Data[3]  = m03;
        m_Data[4]  = m10;
        m_Data[5]  = m11;
        m_Data[6]  = m12;
        m_Data[7]  = m13;
        m_Data[8]  = m20;
        m_Data[9]  = m21;
        m_Data[10] = m22;
        m_Data[11] = m23;
        m_Data[12] = m30;
        m_Data[13] = m31;
        m_Data[14] = m32;
        m_Data[15] = m33;
    }
    explicit Matrix4(const float* data) {
        for (int i = 0; i < 16; ++i)
            m_Data[i] = data[i];
    }

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
    static Matrix4 Identity() {
        Matrix4 result;
        result.SetIdentity();
        return result;
    }

    static Matrix4 Perspective(float fovDegrees, float aspect, float nearZ, float farZ) {
        switch (RendererAPI::GetType()) {
            case RendererAPI::Type::Vulkan:
                return VulkanPerspective(fovDegrees, aspect, nearZ, farZ);

            case RendererAPI::Type::OpenGL:
                return OpenGLPerspective(fovDegrees, aspect, nearZ, farZ);

            default: return VulkanPerspective(fovDegrees, aspect, nearZ, farZ);
        }
    }

    static Matrix4 Orthographic(float left, float right, float bottom, float top, float nearZ,
                                float farZ) {
        // clang-format off
        float projData[16] = {
            2.0f / (right - left), 0.0f, 0.0f, 0.0f,
            0.0f, -2.0f / (top - bottom), 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f / (farZ - nearZ), 0.0f,
            -(right + left) / (right - left), -(top + bottom) / (top - bottom), -nearZ / (farZ - nearZ), 1.0f
        };
        return Matrix4(projData[0], projData[1], projData[2], projData[3],
                        projData[4], projData[5], projData[6], projData[7],
                        projData[8], projData[9], projData[10], projData[11],
                        projData[12], projData[13], projData[14], projData[15]);
        // clang-format on
    }

    static Matrix4 LookAt(const Vector3& eye, const Vector3& center, const Vector3& up);

    // ===== Transformations =====
    static Matrix4 Translate(const Vector3& translation);

    static Matrix4 Translate(float x, float y, float z) { return Translate(Vector3(x, y, z)); }

    static Matrix4 Rotate(float angleDegrees, const Vector3& axis);

    static Matrix4 Rotate(float angleDegrees, float axisX, float axisY, float axisZ) {
        return Rotate(angleDegrees, Vector3(axisX, axisY, axisZ));
    }

    static Matrix4 Scale(const Vector3& scale);

    static Matrix4 Scale(float x, float y, float z) { return Scale(Vector3(x, y, z)); }

    static Matrix4 Scale(float uniform) { return Scale(Vector3(uniform, uniform, uniform)); }

    // ===== Matrix operations =====
    Matrix4 Inverse();

    Matrix4 Transpose();

    Matrix3 ToMatrix3() const;

    // ===== Decomposition (useful for extracting translation/rotation/scale) =====
    Vector3 GetTranslation() const { return Vector3(m_Data[12], m_Data[13], m_Data[14]); }

    Quaternion GetRotation() const;

    Vector3 GetScale() const;

    // ===== Utility =====
    bool IsIdentity(float tolerance = 1e-6f) const {
        Matrix4 identity;
        for (int i = 0; i < 16; ++i) {
            if (std::abs(m_Data[i] - identity.m_Data[i]) > tolerance) {
                return false;
            }
        }
        return true;
    }

    float Determinant() const;

    // ===== Common transformation matrices =====
    static Matrix4 Translation(float x, float y, float z) { return Translate(x, y, z); }

    static Matrix4 RotationX(float angleDegrees) { return Rotate(angleDegrees, Vector3::Right); }

    static Matrix4 RotationY(float angleDegrees) { return Rotate(angleDegrees, Vector3::Up); }

    static Matrix4 RotationZ(float angleDegrees) { return Rotate(angleDegrees, Vector3::Forward); }

    static Matrix4 TRS(const Vector3& translation, const Quaternion& rotation,
                       const Vector3& scale);

    // ===== Operators =====
    Vector3 operator*(const Vector3& v) const;

    Vector4 operator*(const Vector4& v) const {
        return Vector4(m_Data[0] * v.x + m_Data[1] * v.y + m_Data[2] * v.z + m_Data[3] * v.w,
                       m_Data[4] * v.x + m_Data[5] * v.y + m_Data[6] * v.z + m_Data[7] * v.w,
                       m_Data[8] * v.x + m_Data[9] * v.y + m_Data[10] * v.z + m_Data[11] * v.w,
                       m_Data[12] * v.x + m_Data[13] * v.y + m_Data[14] * v.z + m_Data[15] * v.w);
    }

    Matrix4 operator*(const Matrix4& other) const;

    Matrix4& operator*=(const Matrix4& other) {
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

    static Matrix4 VulkanPerspective(float fov, float aspect, float nearZ, float farZ);

    static Matrix4 OpenGLPerspective(float fov, float aspect, float nearZ, float farZ);

private:
    float m_Data[16];
};

inline std::ostream& operator<<(std::ostream& os, const Matrix4& mat) {
    os << mat.ToString();
    return os;
}

} // namespace CZ

// ===== String formatting for logs =====
// CZ_LOG(LogTemp, Info, "Matrix: {}", mat);     // Defaut
// CZ_LOG(LogTemp, Info, "Matrix: {:c}", mat);   // Compact
// CZ_LOG(LogTemp, Info, "Matrix: {:4}", mat);   // Specific precision
namespace fmt {
inline namespace v10 {
template <> struct formatter<CZ::Matrix4> {
    // Optional format specifiers
    int precision = 2;
    bool compact  = false;

    constexpr auto parse(format_parse_context& ctx) {
        auto it = ctx.begin();

        if (it == ctx.end() || *it == '}') {
            return it;
        }

        if (*it == 'c') {
            compact = true;
            ++it;
        } else if (*it >= '0' && *it <= '9') {
            precision = 0;
            while (it != ctx.end() && *it >= '0' && *it <= '9') {
                precision = precision * 10 + (*it - '0');
                ++it;
            }
        } else {
            throw format_error("invalid format specifier");
        }

        if (it == ctx.end() || *it != '}') {
            throw format_error("invalid format");
        }

        ctx.advance_to(it);
        return it;
    }

    template <typename FormatContext>
    auto format(const CZ::Matrix4& mat, FormatContext& ctx) const {
        if (compact) {
            return fmt::format_to(ctx.out(), "{}", mat.ToStringCompact());
        }
        return fmt::format_to(ctx.out(), "{}", mat.ToString());
    }
};
} // namespace v10
} // namespace fmt