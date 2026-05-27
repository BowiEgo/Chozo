#include <Core/Math/Vector4.hpp>

// Ensure memory layout matches GLM
static_assert(sizeof(glm::vec4) == 16, "glm::vec4 should be 16 bytes");
static_assert(alignof(glm::vec4) == 4, "glm::vec4 should be 4-byte aligned");

namespace CZ {

// ===== Arithmetic operators =====
Vector4 Vector4::operator-() const { return Vector4(-x, -y, -z, -w); }

Vector4 Vector4::operator+(const Vector4& v) const {
    return Vector4(x + v.x, y + v.y, z + v.z, w + v.w);
}

Vector4 Vector4::operator-(const Vector4& v) const {
    return Vector4(x - v.x, y - v.y, z - v.z, w - v.w);
}

Vector4 Vector4::operator*(float s) const { return Vector4(x * s, y * s, z * s, w * s); }

Vector4 Vector4::operator/(float s) const {
    float inv = 1.0f / s;
    return Vector4(x * inv, y * inv, z * inv, w * inv);
}

// ===== Compound assignment operators =====
Vector4& Vector4::operator+=(const Vector4& v) {
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;
    return *this;
}

Vector4& Vector4::operator-=(const Vector4& v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;
    return *this;
}

Vector4& Vector4::operator*=(float s) {
    x *= s;
    y *= s;
    z *= s;
    w *= s;
    return *this;
}

Vector4& Vector4::operator/=(float s) {
    float inv = 1.0f / s;
    x *= inv;
    y *= inv;
    z *= inv;
    w *= inv;
    return *this;
}

// ===== Comparison operators =====
bool Vector4::operator==(const Vector4& v) const {
    return x == v.x && y == v.y && z == v.z && w == v.w;
}

bool Vector4::operator!=(const Vector4& v) const { return !(*this == v); }

// Comparison with tolerance
bool Vector4::Equals(const Vector4& v, float tolerance) const {
    return std::abs(x - v.x) <= tolerance && std::abs(y - v.y) <= tolerance &&
           std::abs(z - v.z) <= tolerance && std::abs(w - v.w) <= tolerance;
}

// ===== Vector operations =====
float Vector4::Dot(const Vector4& v) const { return x * v.x + y * v.y + z * v.z + w * v.w; }

float Vector4::Length() const { return std::sqrt(LengthSquared()); }

float Vector4::LengthSquared() const { return x * x + y * y + z * z + w * w; }

Vector4 Vector4::Normalized() const {
    float len = Length();
    if (len > 0.0f) {
        float inv = 1.0f / len;
        return Vector4(x * inv, y * inv, z * inv, w * inv);
    }
    return *this;
}

void Vector4::Normalize() {
    float len = Length();
    if (len > 0.0f) {
        float inv = 1.0f / len;
        x *= inv;
        y *= inv;
        z *= inv;
        w *= inv;
    }
}

float Vector4::Distance(const Vector4& v) const { return (*this - v).Length(); }

float Vector4::DistanceSquared(const Vector4& v) const { return (*this - v).LengthSquared(); }

// ===== Linear interpolation =====
Vector4 Vector4::Lerp(const Vector4& target, float t) const {
    return Vector4(x + (target.x - x) * t, y + (target.y - y) * t, z + (target.z - z) * t,
                   w + (target.w - w) * t);
}

// ===== Homogeneous operations =====
Vector3 Vector4::ToVector3() const {
    if (w != 0.0f) {
        float invW = 1.0f / w;
        return Vector3(x * invW, y * invW, z * invW);
    }
    return Vector3(x, y, z);
}

bool Vector4::IsHomogeneous() const { return w != 0.0f; }
} // namespace CZ