#pragma once

#include "Matrix4.h"
#include "Vector3.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <spdlog/fmt/bundled/format.h>

// Ensure memory layout matches GLM
static_assert(sizeof(glm::quat) == 16, "glm::quat should be 16 bytes");
static_assert(alignof(glm::quat) == 4, "glm::quat should be 4-byte aligned");

class FQuaternion {
public:
    // ===== Constructors =====
    constexpr FQuaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}

    constexpr FQuaternion(float X, float Y, float Z, float W) : x(X), y(Y), z(Z), w(W) {}

    // Implicit construction from GLM
    FQuaternion(const glm::quat& q) : x(q.x), y(q.y), z(q.z), w(q.w) {}

    // Construction from array
    explicit FQuaternion(const float* data) : x(data[0]), y(data[1]), z(data[2]), w(data[3]) {}

    // ===== Conversion to GLM =====
    operator glm::quat() const { return glm::quat(w, x, y, z); }

    // ===== Data access =====
    const float* Data() const { return &x; }
    float* Data() { return &x; }

    float& operator[](int index) { return (&x)[index]; }
    const float& operator[](int index) const { return (&x)[index]; }

    // ===== Factory methods =====
    static FQuaternion Identity() { return FQuaternion(0, 0, 0, 1); }

    // Create from axis-angle (in degrees)
    static FQuaternion FromAxisAngle(const FVector3& axis, float degrees) {
        return FQuaternion(glm::angleAxis(glm::radians(degrees), axis.ToGLM()));
    }

    // Create from Euler angles (in degrees, applied in order: pitch (X), yaw (Y), roll (Z))
    static FQuaternion FromEuler(float pitch, float yaw, float roll) {
        return FQuaternion(
            glm::quat(glm::vec3(glm::radians(pitch), glm::radians(yaw), glm::radians(roll))));
    }

    static FQuaternion FromEuler(const FVector3& eulerDegrees) {
        return FromEuler(eulerDegrees.x, eulerDegrees.y, eulerDegrees.z);
    }

    // Create from two direction vectors (rotation from a to b)
    static FQuaternion FromTo(const FVector3& from, const FVector3& to) {
        return FQuaternion(glm::rotation(from.ToGLM(), to.ToGLM()));
    }

    // ===== Basic operations =====
    FQuaternion Conjugated() const { return FQuaternion(-x, -y, -z, w); }

    FQuaternion Inverse() const { return FQuaternion(glm::inverse(glm::quat(w, x, y, z))); }

    float Length() const { return std::sqrt(x * x + y * y + z * z + w * w); }

    float LengthSquared() const { return x * x + y * y + z * z + w * w; }

    FQuaternion Normalized() const {
        float len = Length();
        if (len > 0.0f) {
            float inv = 1.0f / len;
            return FQuaternion(x * inv, y * inv, z * inv, w * inv);
        }
        return *this;
    }

    void Normalize() {
        float len = Length();
        if (len > 0.0f) {
            float inv = 1.0f / len;
            x *= inv;
            y *= inv;
            z *= inv;
            w *= inv;
        }
    }

    // ===== Operators =====
    FQuaternion operator*(const FQuaternion& q) const {
        return FQuaternion(glm::quat(w, x, y, z) * glm::quat(q.w, q.x, q.y, q.z));
    }

    FQuaternion& operator*=(const FQuaternion& q) {
        *this = *this * q;
        return *this;
    }

    // Rotate a vector by this quaternion
    FVector3 operator*(const FVector3& v) const {
        return FVector3(glm::quat(w, x, y, z) * v.ToGLM());
    }

    // ===== Comparison =====
    bool operator==(const FQuaternion& q) const {
        return x == q.x && y == q.y && z == q.z && w == q.w;
    }

    bool operator!=(const FQuaternion& q) const { return !(*this == q); }

    bool Equals(const FQuaternion& q, float tolerance = 1e-6f) const {
        return std::abs(x - q.x) <= tolerance && std::abs(y - q.y) <= tolerance &&
               std::abs(z - q.z) <= tolerance && std::abs(w - q.w) <= tolerance;
    }

    // ===== Conversions =====
    FVector3 ToEuler() const {
        glm::vec3 euler = glm::eulerAngles(glm::quat(w, x, y, z));
        return FVector3(glm::degrees(euler.x), glm::degrees(euler.y), glm::degrees(euler.z));
    }

    FMatrix4 ToMatrix() const { return FMatrix4(glm::mat4_cast(glm::quat(w, x, y, z))); }

    // ===== Interpolation =====
    FQuaternion Slerp(const FQuaternion& target, float t) const {
        return FQuaternion(glm::slerp(glm::quat(w, x, y, z),
                                      glm::quat(target.w, target.x, target.y, target.z), t));
    }

    FQuaternion NLerp(const FQuaternion& target, float t) const {
        FQuaternion result(x + (target.x - x) * t, y + (target.y - y) * t, z + (target.z - z) * t,
                           w + (target.w - w) * t);
        return result.Normalized();
    }

    // ===== Component access =====
    float& X() { return x; }
    float& Y() { return y; }
    float& Z() { return z; }
    float& W() { return w; }

    const float& X() const { return x; }
    const float& Y() const { return y; }
    const float& Z() const { return z; }
    const float& W() const { return w; }

    // ===== Debug string =====
    std::string ToString() const {
        return fmt::format("[{:6.3f}, {:6.3f}, {:6.3f}, {:6.3f}]", x, y, z, w);
    }

public:
    float x, y, z, w;
};

// ===== Global operators =====
inline FQuaternion operator*(float s, const FQuaternion& q) {
    return FQuaternion(q.x * s, q.y * s, q.z * s, q.w * s);
}

// ===== Common constants =====
static const FQuaternion QuatIdentity(0.0f, 0.0f, 0.0f, 1.0f);

// ===== String formatting for logs =====
template <> struct fmt::formatter<FQuaternion> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template <typename FormatContext> auto format(const FQuaternion& q, FormatContext& ctx) const {
        return fmt::format_to(ctx.out(), "{}", q.ToString());
    }
};