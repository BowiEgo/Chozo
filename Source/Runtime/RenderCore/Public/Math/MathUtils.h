#pragma once

#include "Matrix4.h"
#include "Quaternion.h"
#include "Vector2.h"
#include "Vector3.h"

#include <cmath>

namespace ChozoUtils::Math {

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

inline void DecomposeTransform(const FMatrix4& transform, FVector3& translation,
                               FQuaternion& rotation, FVector3& scale) {
    translation = transform.GetTranslation();

    scale.x = FVector3(transform[0][0], transform[1][0], transform[2][0]).Length();
    scale.y = FVector3(transform[0][1], transform[1][1], transform[2][1]).Length();
    scale.z = FVector3(transform[0][2], transform[1][2], transform[2][2]).Length();

    FMatrix3 rotationMatrix(
        transform[0][0] / scale.x, transform[0][1] / scale.y, transform[0][2] / scale.z,
        transform[1][0] / scale.x, transform[1][1] / scale.y, transform[1][2] / scale.z,
        transform[2][0] / scale.x, transform[2][1] / scale.y, transform[2][2] / scale.z);

    rotation = FQuaternion::FromMatrix(rotationMatrix);
}

} // namespace ChozoUtils::Math