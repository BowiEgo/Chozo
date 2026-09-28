#pragma once

#include <Core/Math/MathUtils.hpp>
#include <Core/Math/Matrix4.hpp>
#include <Core/Math/Quaternion.hpp>
#include <Core/Math/Vector3.hpp>

#include <Runtime/RenderCore/Components/TransformParams.hpp>

namespace CZ {

struct TransformComponent {
    // Value semantics: the transform owns its own data, so there is nothing to free and the
    // ECS storage may copy or move the component freely.
    Vector3 Translation = Vector3::Zero;
    Quaternion Rotation = Quaternion::Identity();
    Vector3 Scale       = Vector3::One;

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

    explicit TransformComponent(const TransformParams& params) { SetTransformParams(params); }

    TransformComponent operator*(const TransformComponent& other) const;

    // ===== Property Getters =====
    Vector3 GetTranslation() const { return Translation; }
    Quaternion GetRotation() const { return Rotation; }
    Vector3 GetScale() const { return Scale; }
    Vector3 GetForward() const { return Rotation * Vector3::Forward; }
    Vector3 GetRight() const { return Rotation * Vector3::Right; }
    Vector3 GetUp() const { return Rotation * Vector3::Up; }
    Vector3 GetRotationEuler() const { return Rotation.ToEuler(); }
    Matrix4 GetLocalMatrix() const {
        // T * R * S
        return Matrix4::Translate(Translation) * Rotation.ToMatrix4() * Matrix4::Scale(Scale);
    }

    Matrix3 GetNormalMatrix(const Matrix4& model) const {
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
