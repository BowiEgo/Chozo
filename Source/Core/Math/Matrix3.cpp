#include <Core/Math/Matrix3.hpp>

#include "MathGLMInterop.hpp"

static_assert(sizeof(glm::mat3) == 36, "glm::mat3 should be 36 bytes");
static_assert(alignof(glm::mat3) == 4, "glm::mat3 should be 4-byte aligned");

namespace CZ {

Matrix3 Matrix3::operator*(const Matrix3& other) const {
    return FromGLM(ToGLM(*this) * ToGLM(other));
}

Vector3 Matrix3::operator*(const Vector3& v) const {
    return FromGLM(ToGLM(*this) * glm::vec3(v.x, v.y, v.z));
}

Matrix3 Matrix3::Transpose() const { return FromGLM(glm::transpose(ToGLM(*this))); }

float Matrix3::Determinant() const { return glm::determinant(ToGLM(*this)); }

Matrix3 Matrix3::Inverse() const { return FromGLM(glm::inverse(ToGLM(*this))); }

void Matrix3::SetIdentity() {
    memset(m_Data, 0, 36);
    m_Data[0] = m_Data[4] = m_Data[8] = 1.0f;
}

std::string Matrix3::ToString() const {
    return fmt::format("[{:6.3f} {:6.3f} {:6.3f}]\n"
                       "[{:6.3f} {:6.3f} {:6.3f}]\n"
                       "[{:6.3f} {:6.3f} {:6.3f}]",
                       m_Data[0], m_Data[1], m_Data[2], m_Data[3], m_Data[4], m_Data[5], m_Data[6],
                       m_Data[7], m_Data[8]);
}

Matrix3 Matrix3::Identity() {
    Matrix3 result;
    result.SetIdentity();
    return result;
}

Matrix3 Matrix3::RotationX(float angleDegrees) {
    float rad = glm::radians(angleDegrees);
    float c   = cosf(rad);
    float s   = sinf(rad);
    return Matrix3(1, 0, 0, 0, c, -s, 0, s, c);
}

Matrix3 Matrix3::RotationY(float angleDegrees) {
    float rad = glm::radians(angleDegrees);
    float c   = cosf(rad);
    float s   = sinf(rad);
    return Matrix3(c, 0, s, 0, 1, 0, -s, 0, c);
}

Matrix3 Matrix3::RotationZ(float angleDegrees) {
    float rad = glm::radians(angleDegrees);
    float c   = cosf(rad);
    float s   = sinf(rad);
    return Matrix3(c, -s, 0, s, c, 0, 0, 0, 1);
}

Matrix3 Matrix3::FromAxisAngle(const Vector3& axis, float angleDegrees) {
    float rad        = glm::radians(angleDegrees);
    glm::mat3 glmMat = glm::mat3_cast(glm::angleAxis(rad, ToGLM(axis)));
    return FromGLM(glmMat);
}

} // namespace CZ