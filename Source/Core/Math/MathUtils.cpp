#include <Core/Math/MathUtils.hpp>

namespace CZ::MathUtils {

float Clamp(float value, float minValue, float maxValue) {
    return glm::clamp(value, minValue, maxValue);
}

void DecomposeTransform(const Matrix4& transform, Vector3& translation, Quaternion& rotation,
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