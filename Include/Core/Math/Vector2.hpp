#pragma once

namespace CZ {

class Vector2 {
public:
    // ===== Constructors =====
    constexpr Vector2() : x(0.0f), y(0.0f) {}
    constexpr Vector2(float X, float Y) : x(X), y(Y) {}
    explicit Vector2(const float* data) : x(data[0]), y(data[1]) {}

    // ===== Data access =====
    const float* Data() const { return &x; }
    float* Data() { return &x; }

    float& operator[](int index) { return (&x)[index]; }
    const float& operator[](int index) const { return (&x)[index]; }

    // ===== Arithmetic operators =====
    Vector2 operator-() const;
    Vector2 operator+(const Vector2& v) const;
    Vector2 operator-(const Vector2& v) const;
    Vector2 operator*(float s) const;
    Vector2 operator/(float s) const;

    // ===== Compound assignment operators =====
    Vector2& operator+=(const Vector2& v);
    Vector2& operator-=(const Vector2& v);
    Vector2& operator*=(float s);
    Vector2& operator/=(float s);

    // ===== Comparison operators =====
    bool operator==(const Vector2& v) const;
    bool operator!=(const Vector2& v) const;

    bool Equals(const Vector2& v, float tolerance = 1e-6f) const;
    float Dot(const Vector2& v) const;
    float Cross(const Vector2& v) const;
    float Length() const;
    float LengthSquared() const;
    Vector2 Normalized() const;
    void Normalize();
    float Distance(const Vector2& v) const;
    float DistanceSquared(const Vector2& v) const;
    Vector2 Lerp(const Vector2& target, float t) const;
    Vector2 Perpendicular() const;
    Vector2 PerpendicularReverse() const;
    Vector2 ProjectOnto(const Vector2& onto) const;
    Vector2 Reflect(const Vector2& normal) const;

    // ===== Component access =====
    float& X() { return x; }
    float& Y() { return y; }

    const float& X() const { return x; }
    const float& Y() const { return y; }

public:
    float x, y;

    // ===== Common vector constants =====
    static const Vector2 Zero;
    static const Vector2 One;
    static const Vector2 Up;
    static const Vector2 Down;
    static const Vector2 Right;
    static const Vector2 Left;
};

// ===== Global operators =====
inline Vector2 operator*(float s, const Vector2& v) { return v * s; }

inline const Vector2 Vector2::Zero(0.0f, 0.0f);
inline const Vector2 Vector2::One(1.0f, 1.0f);
inline const Vector2 Vector2::Up(0.0f, 1.0f);
inline const Vector2 Vector2::Down(0.0f, -1.0f);
inline const Vector2 Vector2::Right(1.0f, 0.0f);
inline const Vector2 Vector2::Left(-1.0f, 0.0f);

} // namespace CZ

namespace std {
template <> struct hash<CZ::Vector2> {
    size_t operator()(const CZ::Vector2& v) const noexcept {
        size_t seed = 0;
        seed ^= hash<float>{}(v.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= hash<float>{}(v.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};
} // namespace std