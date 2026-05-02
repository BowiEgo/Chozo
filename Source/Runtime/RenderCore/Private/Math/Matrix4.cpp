#include "Matrix4.h"

#include "Matrix3.h"
#include "Quaternion.h"

FMatrix4::FMatrix4(const FMatrix3& mat3) {
    const float* m3 = mat3.Data();
    m_Data[0]       = m3[0];
    m_Data[1]       = m3[1];
    m_Data[2]       = m3[2];
    m_Data[3]       = 0.0f;

    m_Data[4] = m3[3];
    m_Data[5] = m3[4];
    m_Data[6] = m3[5];
    m_Data[7] = 0.0f;

    m_Data[8]  = m3[6];
    m_Data[9]  = m3[7];
    m_Data[10] = m3[8];
    m_Data[11] = 0.0f;

    m_Data[12] = 0.0f;
    m_Data[13] = 0.0f;
    m_Data[14] = 0.0f;
    m_Data[15] = 1.0f;
}

FMatrix3 FMatrix4::ToMatrix3() const {
    glm::mat4 glmMat  = glm::make_mat4(m_Data);
    glm::mat3 glmMat3 = glm::mat3(glmMat);
    return FMatrix3(glmMat3);
}

FQuaternion FMatrix4::GetRotation() const {
    glm::mat4 glmMat  = *this;
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