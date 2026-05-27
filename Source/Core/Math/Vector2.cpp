#include <Core/Math/Vector2.hpp>

// Ensure memory layout matches GLM
static_assert(sizeof(glm::vec2) == 8, "glm::vec2 should be 8 bytes");
static_assert(alignof(glm::vec2) == 4, "glm::vec2 should be 4-byte aligned");

namespace CZ {
// ===== Arithmetic operators =====
Vector2 Vector2::operator-() const { return Vector2(-x, -y); }
Vector2 Vector2::operator+(const Vector2& v) const { return Vector2(x + v.x, y + v.y); }
Vector2 Vector2::operator-(const Vector2& v) const { return Vector2(x - v.x, y - v.y); }
Vector2 Vector2::operator*(float s) const { return Vector2(x * s, y * s); }
Vector2 Vector2::operator/(float s) const {
    float inv = 1.0f / s;
    return Vector2(x * inv, y * inv);
}

// ===== Compound assignment operators =====
Vector2& Vector2::operator+=(const Vector2& v) {
    x += v.x;
    y += v.y;
    return *this;
}

Vector2& Vector2::operator-=(const Vector2& v) {
    x -= v.x;
    y -= v.y;
    return *this;
}

Vector2& Vector2::operator*=(float s) {
    x *= s;
    y *= s;
    return *this;
}

Vector2& Vector2::operator/=(float s) {
    float inv = 1.0f / s;
    x *= inv;
    y *= inv;
    return *this;
}

// ===== Comparison operators =====
bool Vector2::operator==(const Vector2& v) const { return x == v.x && y == v.y; }
bool Vector2::operator!=(const Vector2& v) const { return !(*this == v); }

// Comparison with tolerance
bool Vector2::Equals(const Vector2& v, float tolerance) const {
    return std::abs(x - v.x) <= tolerance && std::abs(y - v.y) <= tolerance;
}

// ===== Vector operations =====
float Vector2::Dot(const Vector2& v) const { return x * v.x + y * v.y; }

// 2D cross product (returns scalar, representing the area of the parallelogram)
float Vector2::Cross(const Vector2& v) const { return x * v.y - y * v.x; }

float Vector2::Length() const { return std::sqrt(LengthSquared()); }

float Vector2::LengthSquared() const { return x * x + y * y; }

Vector2 Vector2::Normalized() const {
    float len = Length();
    if (len > 0.0f) {
        float inv = 1.0f / len;
        return Vector2(x * inv, y * inv);
    }
    return *this;
}

void Vector2::Normalize() {
    float len = Length();
    if (len > 0.0f) {
        float inv = 1.0f / len;
        x *= inv;
        y *= inv;
    }
}

float Vector2::Distance(const Vector2& v) const { return (*this - v).Length(); }

float Vector2::DistanceSquared(const Vector2& v) const { return (*this - v).LengthSquared(); }

// ===== Linear interpolation =====
Vector2 Vector2::Lerp(const Vector2& target, float t) const {
    return Vector2(x + (target.x - x) * t, y + (target.y - y) * t);
}

// ===== Perpendicular vectors =====
Vector2 Vector2::Perpendicular() const { return Vector2(-y, x); }

Vector2 Vector2::PerpendicularReverse() const { return Vector2(y, -x); }

// ===== Projection =====
Vector2 Vector2::ProjectOnto(const Vector2& onto) const {
    float d = onto.Dot(onto);
    if (d > 0.0f) {
        return onto * (Dot(onto) / d);
    }
    return Vector2(0, 0);
}

// ===== Reflection =====
Vector2 Vector2::Reflect(const Vector2& normal) const {
    return *this - normal * (2.0f * Dot(normal));
}
} // namespace CZ