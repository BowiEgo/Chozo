#pragma once

#include "MathUtils.h"
#include "Matrix4.h"
#include "Quaternion.h"
#include "Vector3.h"

struct FTransformComponent {
    FVector3 Translation = FVector3::Zero;
    FQuaternion Rotation = FQuaternion::Identity();
    FVector3 Scale = FVector3::One;

    // ===== Optional dirty flag (for ECS synchronization optimization) =====
    mutable uint32_t Revision = 0;

    FTransformComponent() = default;

    explicit FTransformComponent(const FVector3& translation) : Translation(translation) {}

    FTransformComponent(const FVector3& translation, const FQuaternion& rotation,
                        const FVector3& scale)
        : Translation(translation), Rotation(rotation), Scale(scale) {}

    explicit FTransformComponent(const FMatrix4& matrix) { SetMatrix(matrix); }

    FMatrix4 GetMatrix() const {
        // T * R * S
        FMatrix4 translationMatrix = FMatrix4::Translate(Translation);
        FMatrix4 rotationMatrix = Rotation.ToMatrix4();
        FMatrix4 scaleMatrix = FMatrix4::Scale(Scale);

        return translationMatrix * rotationMatrix * scaleMatrix;
    }

    void SetMatrix(const FMatrix4& matrix) {
        ChozoUtils::Math::DecomposeTransform(matrix, Translation, Rotation, Scale);
        Revision++;
    }

    void SetTranslation(const FVector3& translation) {
        if (Translation != translation) {
            Translation = translation;
            Revision++;
        }
    }
    void SetRotation(const FQuaternion& rotation) {
        if (Rotation != rotation) {
            Rotation = rotation;
            Revision++;
        }
    }
    void SetRotationEuler(const FVector3& eulerDegrees) {
        SetRotation(FQuaternion::FromEuler(eulerDegrees));
    }
    void SetScale(const FVector3& scale) {
        if (Scale != scale) {
            Scale = scale;
            Revision++;
        }
    }

    FVector3 GetForward() const { return Rotation * FVector3::Forward; }
    FVector3 GetRight() const { return Rotation * FVector3::Right; }
    FVector3 GetUp() const { return Rotation * FVector3::Up; }
    FVector3 GetRotationEuler() const { return Rotation.ToEuler(); }

    FTransformComponent operator*(const FTransformComponent& other) const {
        FTransformComponent result;
        result.Scale = Scale * other.Scale;
        result.Rotation = Rotation * other.Rotation;
        result.Translation = Translation + Rotation * (Scale * other.Translation);
        return result;
    }

    FVector3 TransformPoint(const FVector3& point) const {
        return Translation + Rotation * (Scale * point);
    }
    FVector3 TransformDirection(const FVector3& direction) const { return Rotation * direction; }
    FVector3 TransformVector(const FVector3& vector) const { return Rotation * (Scale * vector); }

    FTransformComponent Inverse() const {
        FTransformComponent result;
        result.Rotation = Rotation.Conjugated();
        result.Scale = FVector3(1.0f / Scale.x, 1.0f / Scale.y, 1.0f / Scale.z);
        result.Translation = -(result.Rotation * (Translation * result.Scale));
        return result;
    }

    FTransformComponent Lerp(const FTransformComponent& target, float t) const {
        FTransformComponent result;
        result.Translation = Translation.Lerp(target.Translation, t);
        result.Rotation = Rotation.Slerp(target.Rotation, t);
        result.Scale = Scale.Lerp(target.Scale, t);
        return result;
    }

    static const FTransformComponent Identity;
};
