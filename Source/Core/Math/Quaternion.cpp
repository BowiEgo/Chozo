#include <Core/Math/Quaternion.hpp>

#include "MathGLMInterop.hpp"

// Ensure memory layout matches GLM
static_assert(sizeof(glm::quat) == 16, "glm::quat should be 16 bytes");
static_assert(alignof(glm::quat) == 4, "glm::quat should be 4-byte aligned");

namespace CZ {

Quaternion& Quaternion::operator*=(const Quaternion& q) {
    *this = *this * q;
    return *this;
}

Vector3 Quaternion::operator*(const Vector3& v) const {
    return FromGLM(glm::quat(w, x, y, z) * ToGLM(v));
}

bool Quaternion::operator==(const Quaternion& q) const {
    return x == q.x && y == q.y && z == q.z && w == q.w;
}
bool Quaternion::operator!=(const Quaternion& q) const { return !(*this == q); }

Quaternion Quaternion::Identity() { return Quaternion(0.0f, 0.0f, 0.0f, 1.0f); }

Quaternion Quaternion::FromAxisAngle(const Vector3& axis, float degrees) {
    return FromGLM(glm::angleAxis(glm::radians(degrees), ToGLM(axis)));
}

Quaternion Quaternion::FromEuler(float pitch, float yaw, float roll) {
    glm::quat q = glm::quat(glm::vec3(glm::radians(pitch), glm::radians(yaw), glm::radians(roll)));
    return Quaternion(q.x, q.y, q.z, q.w);
}

Quaternion Quaternion::FromEuler(const Vector3& eulerDegrees) {
    return FromEuler(eulerDegrees.x, eulerDegrees.y, eulerDegrees.z);
}

Quaternion Quaternion::FromTo(const Vector3& from, const Vector3& to) {
    return FromGLM(glm::rotation(ToGLM(from), ToGLM(to)));
}

Quaternion Quaternion::FromMatrix(const Matrix3& matrix) {
    return FromGLM(glm::quat_cast(ToGLM(matrix)));
}

Quaternion Quaternion::FromMatrix(const Matrix4& matrix) {
    return FromGLM(glm::quat_cast(ToGLM(matrix)));
}

Vector3 Quaternion::ToEuler() const {
    glm::vec3 euler = glm::eulerAngles(glm::quat(w, x, y, z));
    return Vector3(glm::degrees(euler.x), glm::degrees(euler.y), glm::degrees(euler.z));
}

Matrix3 Quaternion::ToMatrix3() const { return FromGLM(glm::mat3_cast(glm::quat(w, x, y, z))); }

Matrix4 Quaternion::ToMatrix4() const { return FromGLM(glm::mat4_cast(glm::quat(w, x, y, z))); }

Quaternion Quaternion::Conjugated() const { return Quaternion(-x, -y, -z, w); }

Quaternion Quaternion::Inverse() const { return FromGLM(glm::inverse(glm::quat(w, x, y, z))); }

float Quaternion::Length() const { return std::sqrt(x * x + y * y + z * z + w * w); }

float Quaternion::LengthSquared() const { return x * x + y * y + z * z + w * w; }

Quaternion Quaternion::Normalized() const {
    float len = Length();
    if (len > 0.0f) {
        float inv = 1.0f / len;
        return Quaternion(x * inv, y * inv, z * inv, w * inv);
    }
    return *this;
}

void Quaternion::Normalize() {
    float len = Length();
    if (len > 0.0f) {
        float inv = 1.0f / len;
        x *= inv;
        y *= inv;
        z *= inv;
        w *= inv;
    }
}

Quaternion Quaternion::operator*(const Quaternion& q) const {
    return FromGLM(glm::quat(w, x, y, z) * glm::quat(q.w, q.x, q.y, q.z));
}

Quaternion Quaternion::Slerp(const Quaternion& target, float t) const {
    return FromGLM(
        glm::slerp(glm::quat(w, x, y, z), glm::quat(target.w, target.x, target.y, target.z), t));
}

Quaternion Quaternion::NLerp(const Quaternion& target, float t) const {
    Quaternion result(x + (target.x - x) * t, y + (target.y - y) * t, z + (target.z - z) * t,
                      w + (target.w - w) * t);
    return result.Normalized();
}

std::string Quaternion::ToString() const {
    return fmt::format("[{:6.3f}, {:6.3f}, {:6.3f}, {:6.3f}]", x, y, z, w);
}

} // namespace CZ