#pragma once

#include <functional>

namespace CZ {

class Vector3 {
public:
    // ===== Constructors =====
    constexpr Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
    constexpr Vector3(float X, float Y, float Z) : x(X), y(Y), z(Z) {}
    explicit Vector3(const float* data) : x(data[0]), y(data[1]), z(data[2]) {}

    // ===== Data access =====
    const float* Data() const { return &x; }
    float* Data() { return &x; }

    float& operator[](int index) { return (&x)[index]; }

    const float& operator[](int index) const { return (&x)[index]; }

    // ===== Arithmetic operators =====
    Vector3 operator-() const;
    Vector3 operator+(const Vector3& v) const;
    Vector3 operator-(const Vector3& v) const;
    Vector3 operator*(const Vector3& v) const;
    Vector3 operator/(const Vector3& v) const;
    Vector3& operator*=(const Vector3& v);
    Vector3& operator/=(const Vector3& v);
    Vector3 operator*(float s) const;
    Vector3 operator/(float s) const;

    // ===== Compound assignment operators =====
    Vector3& operator+=(const Vector3& v);
    Vector3& operator-=(const Vector3& v);
    Vector3& operator*=(float s);
    Vector3& operator/=(float s);

    // ===== Comparison operators =====
    bool operator==(const Vector3& v) const;
    bool operator!=(const Vector3& v) const;

    bool Equals(const Vector3& v, float tolerance = 1e-6f) const;
    float Dot(const Vector3& v) const;
    Vector3 Cross(const Vector3& v) const;
    float Length() const;
    float LengthSquared() const;
    Vector3 Normalized() const;
    void Normalize();
    float Distance(const Vector3& v) const;
    float DistanceSquared(const Vector3& v) const;
    Vector3 Lerp(const Vector3& target, float t) const;
    Vector3 ProjectOnto(const Vector3& onto) const;
    Vector3 Reflect(const Vector3& normal) const;

    // ===== Component access =====
    float& X() { return x; }
    float& Y() { return y; }
    float& Z() { return z; }

    const float& X() const { return x; }
    const float& Y() const { return y; }
    const float& Z() const { return z; }

public:
    float x, y, z;

    // ===== Common vector constants =====
    static const Vector3 Zero;
    static const Vector3 One;
    static const Vector3 Up;
    static const Vector3 Down;
    static const Vector3 Right;
    static const Vector3 Left;
    static const Vector3 Forward;
    static const Vector3 Backward;
};

// ===== Global operators =====
inline Vector3 operator*(float s, const Vector3& v) { return v * s; }

inline const Vector3 Vector3::Zero(0.0f, 0.0f, 0.0f);
inline const Vector3 Vector3::One(1.0f, 1.0f, 1.0f);
inline const Vector3 Vector3::Up(0.0f, 1.0f, 0.0f);
inline const Vector3 Vector3::Down(0.0f, -1.0f, 0.0f);
inline const Vector3 Vector3::Right(1.0f, 0.0f, 0.0f);
inline const Vector3 Vector3::Left(-1.0f, 0.0f, 0.0f);
inline const Vector3 Vector3::Forward(0.0f, 0.0f, 1.0f); // OpenGL/Vulkan convention
inline const Vector3 Vector3::Backward(0.0f, 0.0f, -1.0f);

} // namespace CZ

namespace std {
template <> struct hash<CZ::Vector3> {
    size_t operator()(const CZ::Vector3& v) const noexcept {
        size_t seed = 0;
        seed ^= hash<float>{}(v.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= hash<float>{}(v.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= hash<float>{}(v.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};
} // namespace std