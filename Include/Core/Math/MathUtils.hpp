#pragma once

#include <Core/Math/Matrix4.hpp>
#include <Core/Math/Quaternion.hpp>
#include <Core/Math/Vector2.hpp>
#include <Core/Math/Vector3.hpp>

#include <cmath>

namespace CZ::MathUtils {

constexpr float PI         = 3.14159265358979323846f;
constexpr float HALF_PI    = PI * 0.5f;
constexpr float QUARTER_PI = PI * 0.25f;
constexpr float TWO_PI     = PI * 2.0f;

constexpr float ToRadians(float degrees) { return degrees * (PI / 180.0f); }
constexpr float ToDegrees(float radians) { return radians * (180.0f / PI); }

inline float Clamp(float value, float minValue = 0.0f, float maxValue = 1.0f) {
    return glm::clamp(value, minValue, maxValue);
}

inline float Sin(float angle) { return std::sin(angle); }
inline float Cos(float angle) { return std::cos(angle); }
inline float Tan(float angle) { return std::tan(angle); }

inline void DecomposeTransform(const Matrix4& transform, Vector3& translation, Quaternion& rotation,
                               Vector3& scale) {
    translation = transform.GetTranslation();

    scale.x = Vector3(transform[0][0], transform[1][0], transform[2][0]).Length();
    scale.y = Vector3(transform[0][1], transform[1][1], transform[2][1]).Length();
    scale.z = Vector3(transform[0][2], transform[1][2], transform[2][2]).Length();

    Matrix3 rotationMatrix(
        transform[0][0] / scale.x, transform[0][1] / scale.y, transform[0][2] / scale.z,
        transform[1][0] / scale.x, transform[1][1] / scale.y, transform[1][2] / scale.z,
        transform[2][0] / scale.x, transform[2][1] / scale.y, transform[2][2] / scale.z);

    rotation = Quaternion::FromMatrix(rotationMatrix);
}

} // namespace CZ::MathUtils