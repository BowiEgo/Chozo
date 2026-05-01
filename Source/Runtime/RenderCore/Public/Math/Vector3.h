#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

// Ensure memory layout matches GLM
static_assert(sizeof(glm::vec3) == 12, "glm::vec3 should be 12 bytes");
static_assert(alignof(glm::vec3) == 4, "glm::vec3 should be 4-byte aligned");

class FVector3 {
public:
    // ===== Constructors =====
    constexpr FVector3(float InValue = 0.0f) : x(InValue), y(InValue), z(InValue) {}
    constexpr FVector3(float X, float Y, float Z) : x(X), y(Y), z(Z) {}

    // Implicit construction from GLM
    explicit FVector3(const glm::vec3& v) : x(v.x), y(v.y), z(v.z) {}
    // Construction from array
    explicit FVector3(const float* data) : x(data[0]), y(data[1]), z(data[2]) {}

    // ===== Conversion to GLM =====
    operator glm::vec3() const { return glm::vec3(x, y, z); }

    // ===== Data access =====
    const float* Data() const { return &x; }
    float* Data() { return &x; }

    float& operator[](int index) { return (&x)[index]; }

    const float& operator[](int index) const { return (&x)[index]; }

    // ===== Arithmetic operators =====
    FVector3 operator-() const { return FVector3(-x, -y, -z); }

    FVector3 operator+(const FVector3& v) const { return FVector3(x + v.x, y + v.y, z + v.z); }
    FVector3 operator-(const FVector3& v) const { return FVector3(x - v.x, y - v.y, z - v.z); }
    FVector3 operator*(const FVector3& v) const { return FVector3(x * v.x, y * v.y, z * v.z); }
    FVector3 operator/(const FVector3& v) const { return FVector3(x / v.x, y / v.y, z / v.z); }
    FVector3& operator*=(const FVector3& v) {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        return *this;
    }
    FVector3& operator/=(const FVector3& v) {
        x /= v.x;
        y /= v.y;
        z /= v.z;
        return *this;
    }

    FVector3 operator*(float s) const { return FVector3(x * s, y * s, z * s); }
    FVector3 operator/(float s) const {
        float inv = 1.0f / s;
        return FVector3(x * inv, y * inv, z * inv);
    }

    // ===== Compound assignment operators =====
    FVector3& operator+=(const FVector3& v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }
    FVector3& operator-=(const FVector3& v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    FVector3& operator*=(float s) {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }
    FVector3& operator/=(float s) {
        float inv = 1.0f / s;
        x *= inv;
        y *= inv;
        z *= inv;
        return *this;
    }

    // ===== Comparison operators =====
    bool operator==(const FVector3& v) const { return x == v.x && y == v.y && z == v.z; }
    bool operator!=(const FVector3& v) const { return !(*this == v); }

    // Comparison with tolerance
    bool Equals(const FVector3& v, float tolerance = 1e-6f) const {
        return std::abs(x - v.x) <= tolerance && std::abs(y - v.y) <= tolerance &&
               std::abs(z - v.z) <= tolerance;
    }

    // ===== Vector operations =====
    float Dot(const FVector3& v) const { return x * v.x + y * v.y + z * v.z; }

    FVector3 Cross(const FVector3& v) const {
        return FVector3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
    }

    float Length() const { return std::sqrt(LengthSquared()); }

    float LengthSquared() const { return x * x + y * y + z * z; }

    FVector3 Normalized() const {
        float len = Length();
        if (len > 0.0f) {
            float inv = 1.0f / len;
            return FVector3(x * inv, y * inv, z * inv);
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
        }
    }

    float Distance(const FVector3& v) const { return (*this - v).Length(); }

    float DistanceSquared(const FVector3& v) const { return (*this - v).LengthSquared(); }

    // ===== Linear interpolation =====
    FVector3 Lerp(const FVector3& target, float t) const {
        return FVector3(x + (target.x - x) * t, y + (target.y - y) * t, z + (target.z - z) * t);
    }

    // ===== Projection =====
    FVector3 ProjectOnto(const FVector3& onto) const {
        float d = onto.Dot(onto);
        if (d > 0.0f) {
            return onto * (Dot(onto) / d);
        }
        return FVector3(0, 0, 0);
    }

    // ===== Reflection =====
    FVector3 Reflect(const FVector3& normal) const { return *this - normal * (2.0f * Dot(normal)); }

    // ===== Component access =====
    float& X() { return x; }
    float& Y() { return y; }
    float& Z() { return z; }

    const float& X() const { return x; }
    const float& Y() const { return y; }
    const float& Z() const { return z; }

    // ===== Get vector components =====
    glm::vec3 ToGLM() const { return glm::vec3(x, y, z); }

public:
    float x, y, z;

    // ===== Common vector constants =====
    static const FVector3 Zero;
    static const FVector3 One;
    static const FVector3 Up;
    static const FVector3 Down;
    static const FVector3 Right;
    static const FVector3 Left;
    static const FVector3 Forward;
    static const FVector3 Backward;
};

// ===== Global operators =====
inline FVector3 operator*(float s, const FVector3& v) { return v * s; }

inline const FVector3 FVector3::Up(0.0f, 1.0f, 0.0f);
inline const FVector3 FVector3::Down(0.0f, -1.0f, 0.0f);
inline const FVector3 FVector3::Right(1.0f, 0.0f, 0.0f);
inline const FVector3 FVector3::Left(-1.0f, 0.0f, 0.0f);
inline const FVector3 FVector3::Forward(0.0f, 0.0f, -1.0f);
inline const FVector3 FVector3::Backward(0.0f, 0.0f, 1.0f);

inline const FVector3 FVector3::Zero(0.0f, 0.0f, 0.0f);
inline const FVector3 FVector3::One(1.0f, 1.0f, 1.0f);

namespace std {
template <> struct hash<FVector3> {
    size_t operator()(const FVector3& v) const noexcept {
        size_t seed = 0;
        seed ^= hash<float>{}(v.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= hash<float>{}(v.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= hash<float>{}(v.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};
} // namespace std