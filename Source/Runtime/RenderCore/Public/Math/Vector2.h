#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

// Ensure memory layout matches GLM
static_assert(sizeof(glm::vec2) == 8, "glm::vec2 should be 8 bytes");
static_assert(alignof(glm::vec2) == 4, "glm::vec2 should be 4-byte aligned");

class FVector2 {
public:
    // ===== Constructors =====
    constexpr FVector2() : x(0.0f), y(0.0f) {}

    constexpr FVector2(float X, float Y) : x(X), y(Y) {}

    // Implicit construction from GLM
    FVector2(const glm::vec2& v) : x(v.x), y(v.y) {}

    // Construction from array
    explicit FVector2(const float* data) : x(data[0]), y(data[1]) {}

    // ===== Conversion to GLM =====
    operator glm::vec2() const { return glm::vec2(x, y); }

    // ===== Data access =====
    const float* Data() const { return &x; }
    float* Data() { return &x; }

    float& operator[](int index) { return (&x)[index]; }

    const float& operator[](int index) const { return (&x)[index]; }

    // ===== Arithmetic operators =====
    FVector2 operator-() const { return FVector2(-x, -y); }

    FVector2 operator+(const FVector2& v) const { return FVector2(x + v.x, y + v.y); }

    FVector2 operator-(const FVector2& v) const { return FVector2(x - v.x, y - v.y); }

    FVector2 operator*(float s) const { return FVector2(x * s, y * s); }

    FVector2 operator/(float s) const {
        float inv = 1.0f / s;
        return FVector2(x * inv, y * inv);
    }

    // ===== Compound assignment operators =====
    FVector2& operator+=(const FVector2& v) {
        x += v.x;
        y += v.y;
        return *this;
    }

    FVector2& operator-=(const FVector2& v) {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    FVector2& operator*=(float s) {
        x *= s;
        y *= s;
        return *this;
    }

    FVector2& operator/=(float s) {
        float inv = 1.0f / s;
        x *= inv;
        y *= inv;
        return *this;
    }

    // ===== Comparison operators =====
    bool operator==(const FVector2& v) const { return x == v.x && y == v.y; }

    bool operator!=(const FVector2& v) const { return !(*this == v); }

    // Comparison with tolerance
    bool Equals(const FVector2& v, float tolerance = 1e-6f) const {
        return std::abs(x - v.x) <= tolerance && std::abs(y - v.y) <= tolerance;
    }

    // ===== Vector operations =====
    float Dot(const FVector2& v) const { return x * v.x + y * v.y; }

    // 2D cross product (returns scalar, representing the area of the parallelogram)
    float Cross(const FVector2& v) const { return x * v.y - y * v.x; }

    float Length() const { return std::sqrt(LengthSquared()); }

    float LengthSquared() const { return x * x + y * y; }

    FVector2 Normalized() const {
        float len = Length();
        if (len > 0.0f) {
            float inv = 1.0f / len;
            return FVector2(x * inv, y * inv);
        }
        return *this;
    }

    void Normalize() {
        float len = Length();
        if (len > 0.0f) {
            float inv = 1.0f / len;
            x *= inv;
            y *= inv;
        }
    }

    float Distance(const FVector2& v) const { return (*this - v).Length(); }

    float DistanceSquared(const FVector2& v) const { return (*this - v).LengthSquared(); }

    // ===== Linear interpolation =====
    FVector2 Lerp(const FVector2& target, float t) const {
        return FVector2(x + (target.x - x) * t, y + (target.y - y) * t);
    }

    // ===== Perpendicular vectors =====
    FVector2 Perpendicular() const { return FVector2(-y, x); }

    FVector2 PerpendicularReverse() const { return FVector2(y, -x); }

    // ===== Projection =====
    FVector2 ProjectOnto(const FVector2& onto) const {
        float d = onto.Dot(onto);
        if (d > 0.0f) {
            return onto * (Dot(onto) / d);
        }
        return FVector2(0, 0);
    }

    // ===== Reflection =====
    FVector2 Reflect(const FVector2& normal) const { return *this - normal * (2.0f * Dot(normal)); }

    // ===== Component access =====
    float& X() { return x; }
    float& Y() { return y; }

    const float& X() const { return x; }
    const float& Y() const { return y; }

    // ===== Get vector components =====
    glm::vec2 ToGLM() const { return glm::vec2(x, y); }

public:
    float x, y;

    // ===== Common vector constants =====
    static const FVector2 Zero;
    static const FVector2 One;
    static const FVector2 Up;
    static const FVector2 Down;
    static const FVector2 Right;
    static const FVector2 Left;
};

// ===== Global operators =====
inline FVector2 operator*(float s, const FVector2& v) { return v * s; }

inline const FVector2 FVector2::Zero(0.0f, 0.0f);
inline const FVector2 FVector2::One(1.0f, 1.0f);
inline const FVector2 FVector2::Up(0.0f, 1.0f);
inline const FVector2 FVector2::Down(0.0f, -1.0f);
inline const FVector2 FVector2::Right(1.0f, 0.0f);
inline const FVector2 FVector2::Left(-1.0f, 0.0f);