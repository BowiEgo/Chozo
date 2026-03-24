#pragma once

#include "MathUtils.h"
#include "Matrix4.h"
#include "Quaternion.h"
#include "TransformParams.h"
#include "Vector3.h"

struct FTransformComponent {
    FTransformParams TransformParams;

    // ===== State =====
    mutable uint32_t Revision = 0;
    mutable bool bIsDirty = true;

    // ===== State Management =====
    void MarkDirty() const {
        bIsDirty = true;
        Revision++;
    }

    void ClearDirty() const { bIsDirty = false; }
    bool IsDirty() const { return bIsDirty; }
    bool IsValid() const { return true; }

    // ===== Constructors =====
    FTransformComponent() = default;
    explicit FTransformComponent(FTransformParams& params) : TransformParams(params) {}

    // ===== Property Getters =====
    FVector3 GetTranslation() { return TransformParams.Translation; }
    FQuaternion GetRotation() { return TransformParams.Rotation; }
    FVector3 GetScale() { return TransformParams.Scale; }
    FVector3 GetTranslation() const { return TransformParams.Translation; }
    FQuaternion GetRotation() const { return TransformParams.Rotation; }
    FVector3 GetScale() const { return TransformParams.Scale; }
    FVector3 GetForward() const { return TransformParams.Rotation * FVector3::Forward; }
    FVector3 GetRight() const { return TransformParams.Rotation * FVector3::Right; }
    FVector3 GetUp() const { return TransformParams.Rotation * FVector3::Up; }
    FVector3 GetRotationEuler() const { return TransformParams.Rotation.ToEuler(); }
    FMatrix4 GetModel() const {
        // T * R * S
        FMatrix4 translationMatrix = FMatrix4::Translate(GetTranslation());
        FMatrix4 rotationMatrix = GetRotation().ToMatrix4();
        FMatrix4 scaleMatrix = FMatrix4::Scale(GetScale());

        return translationMatrix * rotationMatrix * scaleMatrix;
    }

    FMatrix3 GetNormal(const FMatrix4& model) const {

        // glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
        return model.ToMatrix3().Inverse().Transpose();
    }

    // ===== Property Setters =====
    void SetMatrix(const FMatrix4& matrix) {
        auto translation = GetTranslation();
        auto rotation = GetRotation();
        auto scale = GetScale();
        ChozoUtils::Math::DecomposeTransform(matrix, translation, rotation, scale);
        MarkDirty();
    }

    void SetTranslation(const FVector3& translation) {
        if (TransformParams.Translation != translation) {
            TransformParams.Translation = translation;
            MarkDirty();
        }
    }
    void SetRotation(const FQuaternion& rotation) {
        if (TransformParams.Rotation != rotation) {
            TransformParams.Rotation = rotation;
            MarkDirty();
        }
    }
    void SetRotationEuler(const FVector3& eulerDegrees) {
        SetRotation(FQuaternion::FromEuler(eulerDegrees));
    }
    void SetScale(const FVector3& scale) {
        if (TransformParams.Scale != scale) {
            TransformParams.Scale = scale;
            MarkDirty();
        }
    }

    void SetTransformParams(const FTransformParams& params) {
        if (TransformParams == params) return;
        auto* cloned = static_cast<FTransformParams*>(params.Clone());
        TransformParams = *cloned;
        delete cloned;
        MarkDirty();
    }

    FTransformComponent operator*(const FTransformComponent& other) const {
        FTransformComponent result;
        result.SetScale(GetScale() * other.GetScale());
        result.SetRotation(GetRotation() * other.GetRotation());
        result.SetTranslation(GetTranslation() +
                              GetRotation() * (GetScale() * other.GetTranslation()));

        return result;
    }

    FVector3 TransformPoint(const FVector3& point) const {
        return GetTranslation() + GetRotation() * (GetScale() * point);
    }
    FVector3 TransformDirection(const FVector3& direction) const {
        return GetRotation() * direction;
    }
    FVector3 TransformVector(const FVector3& vector) const {
        return GetRotation() * (GetScale() * vector);
    }

    FTransformComponent Inverse() const {
        FTransformComponent result;
        result.SetRotation(GetRotation().Conjugated());
        result.SetScale(FVector3(1.0f / GetScale().x, 1.0f / GetScale().y, 1.0f / GetScale().z));
        result.SetTranslation(-(result.GetRotation() * (GetTranslation() * result.GetScale())));
        return result;
    }

    FTransformComponent Lerp(const FTransformComponent& target, float t) const {
        t = ChozoUtils::Math::Clamp(t, 0.0f, 1.0f);

        FTransformComponent result;
        result.SetTranslation(GetTranslation().Lerp(target.GetTranslation(), t));
        result.SetRotation(GetRotation().Slerp(target.GetRotation(), t));
        result.SetScale(GetScale().Lerp(target.GetScale(), t));
        return result;
    }

    static const FTransformComponent Identity;
};
