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
    MathUtils::DecomposeTransform(matrix, Translation, Rotation, Scale);
    MarkDirty();
}

void TransformComponent::SetTranslation(const Vector3& translation) {
    if (Translation != translation) {
        Translation = translation;
        MarkDirty();
    }
}

void TransformComponent::SetRotation(const Quaternion& rotation) {
    if (Rotation != rotation) {
        Rotation = rotation;
        MarkDirty();
    }
}

void TransformComponent::SetRotationEuler(const Vector3& eulerDegrees) {
    SetRotation(Quaternion::FromEuler(eulerDegrees));
}

void TransformComponent::SetScale(const Vector3& scale) {
    if (Scale != scale) {
        Scale = scale;
        MarkDirty();
    }
}

void TransformComponent::SetTransformParams(const TransformParams params) {
    const TransformParamsObj* incoming = InternalHandleReader::Unwrap(params);
    if (!incoming) return;

    if (Translation == incoming->Translation && Rotation == incoming->Rotation &&
        Scale == incoming->Scale) {
        return;
    }

    Translation = incoming->Translation;
    Rotation    = incoming->Rotation;
    Scale       = incoming->Scale;

    MarkDirty();
}

Vector3 TransformComponent::TransformPoint(const Vector3& point) const {
    return Translation + Rotation * (Scale * point);
}

Vector3 TransformComponent::TransformDirection(const Vector3& direction) const {
    return Rotation * direction;
}

Vector3 TransformComponent::TransformVector(const Vector3& vector) const {
    return Rotation * (Scale * vector);
}

TransformComponent TransformComponent::Inverse() const {
    TransformComponent result;
    result.SetRotation(Rotation.Conjugated());
    result.SetScale(Vector3(1.0f / Scale.x, 1.0f / Scale.y, 1.0f / Scale.z));
    result.SetTranslation(-(result.GetRotation() * (Translation * result.GetScale())));
    return result;
}

TransformComponent TransformComponent::Lerp(const TransformComponent& target, float t) const {
    t = MathUtils::Clamp(t, 0.0f, 1.0f);

    TransformComponent result;
    result.SetTranslation(Translation.Lerp(target.GetTranslation(), t));
    result.SetRotation(Rotation.Slerp(target.GetRotation(), t));
    result.SetScale(Scale.Lerp(target.GetScale(), t));
    return result;
}

} // namespace CZ
