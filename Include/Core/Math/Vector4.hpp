#pragma once

#include <Core/Math/Vector3.hpp>

namespace CZ {

class Vector4 {
public:
    // ===== Constructors =====
    constexpr Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    constexpr Vector4(float X, float Y, float Z, float W) : x(X), y(Y), z(Z), w(W) {}
    explicit Vector4(const Vector3& v, float W = 1.0f) : x(v.x), y(v.y), z(v.z), w(W) {}
    explicit Vector4(const float* data) : x(data[0]), y(data[1]), z(data[2]), w(data[3]) {}

    // ===== Data access =====
    const float* Data() const { return &x; }
    float* Data() { return &x; }
    float& operator[](int index) { return (&x)[index]; }
    const float& operator[](int index) const { return (&x)[index]; }

    // ===== Arithmetic operators =====
    Vector4 operator-() const;
    Vector4 operator+(const Vector4& v) const;
    Vector4 operator-(const Vector4& v) const;
    Vector4 operator*(float s) const;
    Vector4 operator/(float s) const;

    // ===== Compound assignment operators =====
    Vector4& operator+=(const Vector4& v);
    Vector4& operator-=(const Vector4& v);
    Vector4& operator*=(float s);
    Vector4& operator/=(float s);

    // ===== Comparison operators =====
    bool operator==(const Vector4& v) const;
    bool operator!=(const Vector4& v) const;

    bool Equals(const Vector4& v, float tolerance = 1e-6f) const;
    float Dot(const Vector4& v) const;
    float Length() const;
    float LengthSquared() const;
    Vector4 Normalized() const;
    void Normalize();
    float Distance(const Vector4& v) const;
    float DistanceSquared(const Vector4& v) const;
    Vector4 Lerp(const Vector4& target, float t) const;
    Vector3 ToVector3() const;
    bool IsHomogeneous() const;

    // ===== Component access =====
    float& X() { return x; }
    float& Y() { return y; }
    float& Z() { return z; }
    float& W() { return w; }

    const float& X() const { return x; }
    const float& Y() const { return y; }
    const float& Z() const { return z; }
    const float& W() const { return w; }

public:
    float x, y, z, w;

    // ===== Common vector constants =====
    static const Vector4 Zero;
    static const Vector4 One;
    static const Vector4 UnitX;
    static const Vector4 UnitY;
    static const Vector4 UnitZ;
    static const Vector4 UnitW;
};

// ===== Global operators =====
inline Vector4 operator*(float s, const Vector4& v) { return v * s; }

// ===== Common vector constants definitions =====
inline const Vector4 Vector4::Zero(0.0f, 0.0f, 0.0f, 0.0f);
inline const Vector4 Vector4::One(1.0f, 1.0f, 1.0f, 1.0f);
inline const Vector4 Vector4::UnitX(1.0f, 0.0f, 0.0f, 0.0f);
inline const Vector4 Vector4::UnitY(0.0f, 1.0f, 0.0f, 0.0f);
inline const Vector4 Vector4::UnitZ(0.0f, 0.0f, 1.0f, 0.0f);
inline const Vector4 Vector4::UnitW(0.0f, 0.0f, 0.0f, 1.0f);

} // namespace CZ

namespace std {
template <> struct hash<CZ::Vector4> {
    size_t operator()(const CZ::Vector4& v) const noexcept {
        size_t seed = 0;
        seed ^= hash<float>{}(v.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= hash<float>{}(v.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= hash<float>{}(v.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= hash<float>{}(v.w) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};
} // namespace std
