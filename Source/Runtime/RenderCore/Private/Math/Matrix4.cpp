#include "Matrix4.h"

#include "Matrix3.h"
#include "Quaternion.h"

FMatrix3 FMatrix4::ToMatrix3() const {
    glm::mat4 glmMat = glm::make_mat4(m_Data);
    glm::mat3 glmMat3 = glm::mat3(glmMat);
    return FMatrix3(glmMat3);
}

FQuaternion FMatrix4::GetRotation() const {
    glm::mat4 glmMat = *this;
    glm::quat glmQuat = glm::quat_cast(glmMat);
    return FQuaternion(glmQuat);
}

FMatrix4 FMatrix4::TRS(const FVector3& translation, const FQuaternion& rotation,
                       const FVector3& scale) {
    glm::mat4 glmMat = glm::translate(glm::mat4(1.0f), translation.ToGLM()) *
                       glm::mat4_cast(glm::quat(rotation.w, rotation.x, rotation.y, rotation.z)) *
                       glm::scale(glm::mat4(1.0f), scale.ToGLM());
    return FMatrix4(glmMat);
}