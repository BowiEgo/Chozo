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

float Clamp(float value, float minValue = 0.0f, float maxValue = 1.0f);

inline float Sin(float angle) { return std::sin(angle); }
inline float Cos(float angle) { return std::cos(angle); }
inline float Tan(float angle) { return std::tan(angle); }

void DecomposeTransform(const Matrix4& transform, Vector3& translation, Quaternion& rotation,
                        Vector3& scale);

} // namespace CZ::MathUtils