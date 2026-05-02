#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

// Ensure memory layout matches GLM
static_assert(sizeof(glm::vec4) == 16, "glm::vec4 should be 16 bytes");
static_assert(alignof(glm::vec4) == 4, "glm::vec4 should be 4-byte aligned");

class FVector4 {
public:
    // ===== Constructors =====
    constexpr FVector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}

    constexpr FVector4(float X, float Y, float Z, float W) : x(X), y(Y), z(Z), w(W) {}

    // Construct from FVector3 with optional w
    explicit FVector4(const FVector3& v, float W = 1.0f) : x(v.x), y(v.y), z(v.z), w(W) {}

    // Implicit construction from GLM
    FVector4(const glm::vec4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}

    // Construction from array
    explicit FVector4(const float* data) : x(data[0]), y(data[1]), z(data[2]), w(data[3]) {}

    // ===== Conversion to GLM =====
    operator glm::vec4() const { return glm::vec4(x, y, z, w); }

    // ===== Data access =====
    const float* Data() const { return &x; }
    float* Data() { return &x; }

    float& operator[](int index) { return (&x)[index]; }

    const float& operator[](int index) const { return (&x)[index]; }

    // ===== To String =====
    std::string ToString() const {
        return fmt::format("[{:6.2f} {:6.2f} {:6.2f} {:6.2f}]", x, y, z, w);
    }

    // ===== Arithmetic operators =====
    FVector4 operator-() const { return FVector4(-x, -y, -z, -w); }

    FVector4 operator+(const FVector4& v) const {
        return FVector4(x + v.x, y + v.y, z + v.z, w + v.w);
    }

    FVector4 operator-(const FVector4& v) const {
        return FVector4(x - v.x, y - v.y, z - v.z, w - v.w);
    }

    FVector4 operator*(float s) const { return FVector4(x * s, y * s, z * s, w * s); }

    FVector4 operator/(float s) const {
        float inv = 1.0f / s;
        return FVector4(x * inv, y * inv, z * inv, w * inv);
    }

    // ===== Compound assignment operators =====
    FVector4& operator+=(const FVector4& v) {
        x += v.x;
        y += v.y;
        z += v.z;
        w += v.w;
        return *this;
    }

    FVector4& operator-=(const FVector4& v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        w -= v.w;
        return *this;
    }

    FVector4& operator*=(float s) {
        x *= s;
        y *= s;
        z *= s;
        w *= s;
        return *this;
    }

    FVector4& operator/=(float s) {
        float inv = 1.0f / s;
        x *= inv;
        y *= inv;
        z *= inv;
        w *= inv;
        return *this;
    }

    // ===== Comparison operators =====
    bool operator==(const FVector4& v) const {
        return x == v.x && y == v.y && z == v.z && w == v.w;
    }

    bool operator!=(const FVector4& v) const { return !(*this == v); }

    // Comparison with tolerance
    bool Equals(const FVector4& v, float tolerance = 1e-6f) const {
        return std::abs(x - v.x) <= tolerance && std::abs(y - v.y) <= tolerance &&
               std::abs(z - v.z) <= tolerance && std::abs(w - v.w) <= tolerance;
    }

    // ===== Vector operations =====
    float Dot(const FVector4& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }

    float Length() const { return std::sqrt(LengthSquared()); }

    float LengthSquared() const { return x * x + y * y + z * z + w * w; }

    FVector4 Normalized() const {
        float len = Length();
        if (len > 0.0f) {
            float inv = 1.0f / len;
            return FVector4(x * inv, y * inv, z * inv, w * inv);
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

    float Distance(const FVector4& v) const { return (*this - v).Length(); }

    float DistanceSquared(const FVector4& v) const { return (*this - v).LengthSquared(); }

    // ===== Linear interpolation =====
    FVector4 Lerp(const FVector4& target, float t) const {
        return FVector4(x + (target.x - x) * t, y + (target.y - y) * t, z + (target.z - z) * t,
                        w + (target.w - w) * t);
    }

    // ===== Homogeneous operations =====
    FVector3 ToVector3() const {
        if (w != 0.0f) {
            float invW = 1.0f / w;
            return FVector3(x * invW, y * invW, z * invW);
        }
        return FVector3(x, y, z);
    }

    bool IsHomogeneous() const { return w != 0.0f; }

    // ===== Component access =====
    float& X() { return x; }
    float& Y() { return y; }
    float& Z() { return z; }
    float& W() { return w; }

    const float& X() const { return x; }
    const float& Y() const { return y; }
    const float& Z() const { return z; }
    const float& W() const { return w; }

    // ===== Get vector components =====
    glm::vec4 ToGLM() const { return glm::vec4(x, y, z, w); }

public:
    float x, y, z, w;

    // ===== Common vector constants =====
    static const FVector4 Zero;
    static const FVector4 One;
    static const FVector4 UnitX;
    static const FVector4 UnitY;
    static const FVector4 UnitZ;
    static const FVector4 UnitW;
};

// ===== Global operators =====
inline FVector4 operator*(float s, const FVector4& v) { return v * s; }

// ===== Common vector constants definitions =====
inline const FVector4 FVector4::Zero(0.0f, 0.0f, 0.0f, 0.0f);
inline const FVector4 FVector4::One(1.0f, 1.0f, 1.0f, 1.0f);
inline const FVector4 FVector4::UnitX(1.0f, 0.0f, 0.0f, 0.0f);
inline const FVector4 FVector4::UnitY(0.0f, 1.0f, 0.0f, 0.0f);
inline const FVector4 FVector4::UnitZ(0.0f, 0.0f, 1.0f, 0.0f);
inline const FVector4 FVector4::UnitW(0.0f, 0.0f, 0.0f, 1.0f);

namespace std {
template <> struct hash<FVector4> {
    size_t operator()(const FVector4& v) const noexcept {
        size_t seed = 0;
        seed ^= hash<float>{}(v.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= hash<float>{}(v.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= hash<float>{}(v.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= hash<float>{}(v.w) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};
} // namespace std