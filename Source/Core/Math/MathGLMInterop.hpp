#pragma once

#include <Core/Math/Matrix3.hpp>
#include <Core/Math/Matrix4.hpp>
#include <Core/Math/Quaternion.hpp>
#include <Core/Math/Vector2.hpp>
#include <Core/Math/Vector3.hpp>
#include <Core/Math/Vector4.hpp>

namespace CZ {

// ==================== Matrix3 ====================
inline Matrix3 FromGLM(const glm::mat3& m) { return Matrix3(glm::value_ptr(m)); }

inline glm::mat3 ToGLM(const Matrix3& m) {
    glm::mat3 result;
    std::memcpy(glm::value_ptr(result), m.Data(), 9 * sizeof(float));
    return result;
}

// ==================== Matrix4 ====================
inline Matrix4 FromGLM(const glm::mat4& m) { return Matrix4(glm::value_ptr(m)); }

inline glm::mat4 ToGLM(const Matrix4& m) {
    glm::mat4 result;
    std::memcpy(glm::value_ptr(result), m.Data(), 16 * sizeof(float));
    return result;
}

// ==================== Quaternion ====================
inline Quaternion FromGLM(const glm::quat& q) { return Quaternion(q.x, q.y, q.z, q.w); }

inline glm::quat ToGLM(const Quaternion& q) { return glm::quat(q.w, q.x, q.y, q.z); }

// ==================== Vector2 ====================
inline Vector2 FromGLM(const glm::vec2& v) { return Vector2(v.x, v.y); }

inline glm::vec2 ToGLM(const Vector2& v) { return glm::vec2(v.x, v.y); }

// ==================== Vector3 ====================
inline Vector3 FromGLM(const glm::vec3& v) { return Vector3(v.x, v.y, v.z); }

inline glm::vec3 ToGLM(const Vector3& v) { return glm::vec3(v.x, v.y, v.z); }

// ==================== Vector4 ====================
inline Vector4 FromGLM(const glm::vec4& v) { return Vector4(v.x, v.y, v.z, v.w); }

inline glm::vec4 ToGLM(const Vector4& v) { return glm::vec4(v.x, v.y, v.z, v.w); }

} // namespace CZ