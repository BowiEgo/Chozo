#include <Core/Math/Vector3.hpp>

// Ensure memory layout matches GLM
static_assert(sizeof(glm::vec3) == 12, "glm::vec3 should be 12 bytes");
static_assert(alignof(glm::vec3) == 4, "glm::vec3 should be 4-byte aligned");

namespace CZ {

// ===== Arithmetic operators =====
Vector3 Vector3::operator-() const { return Vector3(-x, -y, -z); }

Vector3 Vector3::operator+(const Vector3& v) const { return Vector3(x + v.x, y + v.y, z + v.z); }
Vector3 Vector3::operator-(const Vector3& v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
Vector3 Vector3::operator*(const Vector3& v) const { return Vector3(x * v.x, y * v.y, z * v.z); }
Vector3 Vector3::operator/(const Vector3& v) const { return Vector3(x / v.x, y / v.y, z / v.z); }
Vector3& Vector3::operator*=(const Vector3& v) {
    x *= v.x;
    y *= v.y;
    z *= v.z;
    return *this;
}
Vector3& Vector3::operator/=(const Vector3& v) {
    x /= v.x;
    y /= v.y;
    z /= v.z;
    return *this;
}

Vector3 Vector3::operator*(float s) const { return Vector3(x * s, y * s, z * s); }
Vector3 Vector3::operator/(float s) const {
    float inv = 1.0f / s;
    return Vector3(x * inv, y * inv, z * inv);
}

// ===== Compound assignment operators =====
Vector3& Vector3::operator+=(const Vector3& v) {
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}
Vector3& Vector3::operator-=(const Vector3& v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
}

Vector3& Vector3::operator*=(float s) {
    x *= s;
    y *= s;
    z *= s;
    return *this;
}
Vector3& Vector3::operator/=(float s) {
    float inv = 1.0f / s;
    x *= inv;
    y *= inv;
    z *= inv;
    return *this;
}

// ===== Comparison operators =====
bool Vector3::operator==(const Vector3& v) const { return x == v.x && y == v.y && z == v.z; }
bool Vector3::operator!=(const Vector3& v) const { return !(*this == v); }

// Comparison with tolerance
bool Vector3::Equals(const Vector3& v, float tolerance) const {
    return std::abs(x - v.x) <= tolerance && std::abs(y - v.y) <= tolerance &&
           std::abs(z - v.z) <= tolerance;
}

// ===== Vector operations =====
float Vector3::Dot(const Vector3& v) const { return x * v.x + y * v.y + z * v.z; }

Vector3 Vector3::Cross(const Vector3& v) const {
    return Vector3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
}

float Vector3::Length() const { return std::sqrt(LengthSquared()); }

float Vector3::LengthSquared() const { return x * x + y * y + z * z; }

Vector3 Vector3::Normalized() const {
    float len = Length();
    if (len > 0.0f) {
        float inv = 1.0f / len;
        return Vector3(x * inv, y * inv, z * inv);
    }
    return *this;
}

void Vector3::Normalize() {
    float len = Length();
    if (len > 0.0f) {
        float inv = 1.0f / len;
        x *= inv;
        y *= inv;
        z *= inv;
    }
}

float Vector3::Distance(const Vector3& v) const { return (*this - v).Length(); }

float Vector3::DistanceSquared(const Vector3& v) const { return (*this - v).LengthSquared(); }

// ===== Linear interpolation =====
Vector3 Vector3::Lerp(const Vector3& target, float t) const {
    return Vector3(x + (target.x - x) * t, y + (target.y - y) * t, z + (target.z - z) * t);
}

// ===== Projection =====
Vector3 Vector3::ProjectOnto(const Vector3& onto) const {
    float d = onto.Dot(onto);
    if (d > 0.0f) {
        return onto * (Dot(onto) / d);
    }
    return Vector3(0, 0, 0);
}

// ===== Reflection =====
Vector3 Vector3::Reflect(const Vector3& normal) const {
    return *this - normal * (2.0f * Dot(normal));
}

} // namespace CZ