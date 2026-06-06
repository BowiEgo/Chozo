#include <Runtime/RenderCore/Components/TransformComponent.hpp>

#include <Core/Log/LogMacros.hpp>

namespace CZ {

TransformComponent TransformComponent::operator*(const TransformComponent& other) const {
    TransformComponent result;
    result.SetScale(GetScale() * other.GetScale());
    result.SetRotation(GetRotation() * other.GetRotation());
    result.SetTranslation(GetTranslation() + GetRotation() * (GetScale() * other.GetTranslation()));

    return result;
}

void TransformComponent::SetMatrix(const Matrix4& matrix) {
    auto translation = GetTranslation();
    auto rotation    = GetRotation();
    auto scale       = GetScale();
    MathUtils::DecomposeTransform(matrix, translation, rotation, scale);
    MarkDirty();
}

void TransformComponent::SetTranslation(const Vector3& translation) {
    if (Params->Translation != translation) {
        Params->Translation = translation;
        MarkDirty();
    }
}
void TransformComponent::SetRotation(const Quaternion& rotation) {
    if (Params->Rotation != rotation) {
        Params->Rotation = rotation;
        MarkDirty();
    }
}
void TransformComponent::SetRotationEuler(const Vector3& eulerDegrees) {
    SetRotation(Quaternion::FromEuler(eulerDegrees));
}
void TransformComponent::SetScale(const Vector3& scale) {
    if (Params->Scale != scale) {
        Params->Scale = scale;
        MarkDirty();
    }
}

void TransformComponent::SetTransformParams(const TransformParams params) {
    if (Params == params) return;
    CZ_CORE_LOG(Trace, "SetTransformParams: translation={:3}, rotation={:3}, scale={:3}",
                params->Translation, params->Rotation, params->Scale);
    auto cloned = params.Clone();
    Params      = cloned;

    cloned.Destroy();
    MarkDirty();
}

Vector3 TransformComponent::TransformPoint(const Vector3& point) const {
    return GetTranslation() + GetRotation() * (GetScale() * point);
}
Vector3 TransformComponent::TransformDirection(const Vector3& direction) const {
    return GetRotation() * direction;
}
Vector3 TransformComponent::TransformVector(const Vector3& vector) const {
    return GetRotation() * (GetScale() * vector);
}

TransformComponent TransformComponent::Inverse() const {
    TransformComponent result;
    result.SetRotation(GetRotation().Conjugated());
    result.SetScale(Vector3(1.0f / GetScale().x, 1.0f / GetScale().y, 1.0f / GetScale().z));
    result.SetTranslation(-(result.GetRotation() * (GetTranslation() * result.GetScale())));
    return result;
}

TransformComponent TransformComponent::Lerp(const TransformComponent& target, float t) const {
    t = MathUtils::Clamp(t, 0.0f, 1.0f);

    TransformComponent result;
    result.SetTranslation(GetTranslation().Lerp(target.GetTranslation(), t));
    result.SetRotation(GetRotation().Slerp(target.GetRotation(), t));
    result.SetScale(GetScale().Lerp(target.GetScale(), t));
    return result;
}
} // namespace CZ