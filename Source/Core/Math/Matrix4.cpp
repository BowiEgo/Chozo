#include <Core/Math/Matrix3.hpp>
#include <Core/Math/Matrix4.hpp>
#include <Core/Math/Quaternion.hpp>

#include "MathGLMInterop.hpp"

static_assert(sizeof(glm::mat4) == 64, "glm::mat4 should be 64 bytes");

namespace CZ {

Matrix4 Matrix4::LookAt(const Vector3& eye, const Vector3& center, const Vector3& up) {
    return FromGLM(glm::lookAt(ToGLM(eye), ToGLM(center), ToGLM(up)));
}

// ===== Transformations =====
Matrix4 Matrix4::Translate(const Vector3& translation) {
    return FromGLM(glm::translate(glm::mat4(1.0f), ToGLM(translation)));
}

Matrix4 Matrix4::Rotate(float angleDegrees, const Vector3& axis) {
    return FromGLM(glm::rotate(glm::mat4(1.0f), glm::radians(angleDegrees), ToGLM(axis)));
}

Matrix4 Matrix4::Scale(const Vector3& scale) {
    return FromGLM(glm::scale(glm::mat4(1.0f), ToGLM(scale)));
}

Vector3 Matrix4::GetScale() const {
    glm::mat4 glmMat = ToGLM(*this);
    // Extract scale from matrix columns
    float scaleX     = glm::length(glm::vec3(glmMat[0]));
    float scaleY     = glm::length(glm::vec3(glmMat[1]));
    float scaleZ     = glm::length(glm::vec3(glmMat[2]));
    return Vector3(scaleX, scaleY, scaleZ);
}

float Matrix4::Determinant() const { return glm::determinant(ToGLM(*this)); }

Vector3 Matrix4::operator*(const Vector3& v) const {
    glm::vec4 result = ToGLM(*this) * glm::vec4(v.x, v.y, v.z, 1.0f);
    return Vector3(result.x, result.y, result.z);
}

Matrix4 Matrix4::operator*(const Matrix4& other) const {
    return FromGLM(ToGLM(*this) * ToGLM(other));
}

Matrix4 Matrix4::VulkanPerspective(float fov, float aspect, float nearZ, float farZ) {
    float tanHalfFov = tanf(glm::radians(fov) * 0.5f);
    return Matrix4(1.0f / (aspect * tanHalfFov), 0, 0, 0, 0, -1.0f / tanHalfFov, 0, 0, 0, 0,
                   farZ / (farZ - nearZ), 1, 0, 0, -(nearZ * farZ) / (farZ - nearZ), 0);
}

Matrix4 Matrix4::OpenGLPerspective(float fov, float aspect, float nearZ, float farZ) {
    return FromGLM(glm::perspective(glm::radians(fov), aspect, nearZ, farZ));
}

// ===== Matrix operations =====
Matrix4 Matrix4::Inverse() { return FromGLM(glm::inverse(ToGLM(*this))); }

Matrix4 Matrix4::Transpose() { return FromGLM(glm::transpose(ToGLM(*this))); }

Matrix3 Matrix4::ToMatrix3() const {
    glm::mat4 glmMat  = glm::make_mat4(m_Data);
    glm::mat3 glmMat3 = glm::mat3(glmMat);
    return FromGLM(glmMat3);
}

Quaternion Matrix4::GetRotation() const {
    glm::mat4 glmMat  = ToGLM(*this);
    glm::quat glmQuat = glm::quat_cast(glmMat);
    return FromGLM(glmQuat);
}

Matrix4 Matrix4::TRS(const Vector3& translation, const Quaternion& rotation, const Vector3& scale) {
    glm::mat4 glmMat = glm::translate(glm::mat4(1.0f), ToGLM(translation)) *
                       glm::mat4_cast(glm::quat(rotation.w, rotation.x, rotation.y, rotation.z)) *
                       glm::scale(glm::mat4(1.0f), ToGLM(scale));
    return FromGLM(glmMat);
}

} // namespace CZ
