#pragma once

#include <glm/glm.hpp>

namespace Chozo::Math {

    constexpr float PI = 3.14159265359f;

    bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
}
