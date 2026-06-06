#pragma once

#include <Core/Math/MathUtils.hpp>
#include <Core/Math/Matrix4.hpp>
#include <Core/Math/Quaternion.hpp>
#include <Core/Math/Vector3.hpp>

#include <Runtime/RenderCore/Components/TransformParams.hpp>

namespace CZ {

struct TransformComponent {
    TransformParams Params;
    Matrix4 WorldMatrix       = Matrix4::Identity();
    Matrix3 WorldNormalMatrix = Matrix3::Identity();

    // ===== State =====
    mutable uint32_t Revision = 0;
    mutable bool bIsDirty     = true;

    // ===== State Management =====
    void MarkDirty() const {
        bIsDirty = true;
        Revision++;
    }

    void ClearDirty() const { bIsDirty = false; }
    bool IsDirty() const { return bIsDirty; }
    bool IsValid() const { return true; }

    // ===== Constructors =====
    TransformComponent() = default;
    explicit TransformComponent(TransformParams params) : Params(params) {}

    TransformComponent operator*(const TransformComponent& other) const;

    // ===== Property Getters =====
    Vector3 GetTranslation() { return Params->Translation; }
    Quaternion GetRotation() { return Params->Rotation; }
    Vector3 GetScale() { return Params->Scale; }
    Vector3 GetTranslation() const { return Params->Translation; }
    Quaternion GetRotation() const { return Params->Rotation; }
    Vector3 GetScale() const { return Params->Scale; }
    Vector3 GetForward() const { return Params->Rotation * Vector3::Forward; }
    Vector3 GetRight() const { return Params->Rotation * Vector3::Right; }
    Vector3 GetUp() const { return Params->Rotation * Vector3::Up; }
    Vector3 GetRotationEuler() const { return Params->Rotation.ToEuler(); }
    Matrix4 GetLocalMatrix() const {
        // T * R * S
        Matrix4 translationMatrix = Matrix4::Translate(GetTranslation());
        Matrix4 rotationMatrix    = GetRotation().ToMatrix4();
        Matrix4 scaleMatrix       = Matrix4::Scale(GetScale());

        return translationMatrix * rotationMatrix * scaleMatrix;
    }

    Matrix3 GetNormalMatrix(const Matrix4& model) const {
        // glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
        return model.ToMatrix3().Inverse().Transpose();
    }

    // ===== Property Setters =====
    void SetMatrix(const Matrix4& matrix);

    void SetTranslation(const Vector3& translation);
    void SetRotation(const Quaternion& rotation);
    void SetRotationEuler(const Vector3& eulerDegrees);
    void SetScale(const Vector3& scale);
    void SetTransformParams(const TransformParams params);

    Vector3 TransformPoint(const Vector3& point) const;
    Vector3 TransformDirection(const Vector3& direction) const;
    Vector3 TransformVector(const Vector3& vector) const;
    TransformComponent Inverse() const;
    TransformComponent Lerp(const TransformComponent& target, float t) const;

    static const TransformComponent Identity;
};

} // namespace CZ
