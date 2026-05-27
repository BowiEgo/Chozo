#pragma once

#include <Core/Math/Matrix3.hpp>
#include <Core/Math/Matrix4.hpp>
#include <Core/Math/Vector3.hpp>

namespace CZ {

class Quaternion {
public:
    // ===== Constructors =====
    constexpr Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
    constexpr Quaternion(float X, float Y, float Z, float W) : x(X), y(Y), z(Z), w(W) {}
    explicit Quaternion(const float* data) : x(data[0]), y(data[1]), z(data[2]), w(data[3]) {}

    // ===== Data access =====
    const float* Data() const { return &x; }
    float* Data() { return &x; }

    float& operator[](int index) { return (&x)[index]; }
    const float& operator[](int index) const { return (&x)[index]; }

    // ===== Arithmetic operators =====
    Quaternion operator*(const Quaternion& q) const;
    Quaternion& operator*=(const Quaternion& q);
    Vector3 operator*(const Vector3& v) const;

    static Quaternion Identity();
    static Quaternion FromAxisAngle(const Vector3& axis, float degrees);
    static Quaternion FromEuler(float pitch, float yaw, float roll);
    static Quaternion FromEuler(const Vector3& eulerDegrees);
    static Quaternion FromTo(const Vector3& from, const Vector3& to);
    static Quaternion FromMatrix(const Matrix3& matrix);
    static Quaternion FromMatrix(const Matrix4& matrix);

    Matrix3 ToMatrix3() const;
    Matrix4 ToMatrix4() const;

    Quaternion Conjugated() const;
    Quaternion Inverse() const;
    float Length() const;
    float LengthSquared() const;
    Quaternion Normalized() const;
    void Normalize();

    Quaternion Slerp(const Quaternion& target, float t) const;
    Quaternion NLerp(const Quaternion& target, float t) const;

    std::string ToString() const;

public:
    float x, y, z, w;
};

// ===== Global operators =====
inline Quaternion operator*(float s, const Quaternion& q) {
    return Quaternion(q.x * s, q.y * s, q.z * s, q.w * s);
}

// ===== Common constants =====
static const Quaternion QuatIdentity(0.0f, 0.0f, 0.0f, 1.0f);

} // namespace CZ

namespace std {
template <> struct hash<CZ::Quaternion> {
    size_t operator()(const CZ::Quaternion& q) const noexcept {
        size_t seed = 0;
        seed ^= hash<float>{}(q.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= hash<float>{}(q.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= hash<float>{}(q.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= hash<float>{}(q.w) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};
} // namespace std

// ===== String formatting for logs =====
namespace fmt {
inline namespace v10 {
template <> struct formatter<CZ::Quaternion> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template <typename FormatContext>
    auto format(const CZ::Quaternion& q, FormatContext& ctx) const {
        return fmt::format_to(ctx.out(), "{}", q.ToString());
    }
};
} // namespace v10
} // namespace fmt