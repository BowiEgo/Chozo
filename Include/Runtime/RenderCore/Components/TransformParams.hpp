#pragma once

#include "Core/Memory/MemoryTypes.hpp"
#include <Runtime/RenderCore/Params.hpp>

#include <Core/Math/MathUtils.hpp>
#include <Core/Math/Matrix4.hpp>
#include <Core/Math/Quaternion.hpp>
#include <Core/Math/Vector3.hpp>

namespace CZ {

struct TransformParamsObj : public Params {
    Vector3 Translation = Vector3::Zero;
    Quaternion Rotation = Quaternion::Identity();
    Vector3 Scale       = Vector3::One;

    TransformParamsObj() = default;

    TransformParamsObj(const Vector3& translation) : Translation(translation) {}

    TransformParamsObj(Vector3 translation, Quaternion rotation, Vector3 scale)
        : Translation(translation), Rotation(rotation), Scale(scale) {}

    TransformParamsObj(const TransformParamsObj& other)
        : Translation(other.Translation), Rotation(other.Rotation), Scale(other.Scale) {}

    // ===== Comparison Operators =====
    bool operator==(const TransformParamsObj* other) {
        if (!other) return false;

        return Translation == other->Translation && Rotation == other->Rotation &&
               Scale == other->Scale;
    }

    bool operator!=(const TransformParamsObj* other) const { return !(this == other); }

    // ===== Params Implementation =====
    virtual TransformParamsObj* Clone() const override {
        return CZ_NEW(MEMORY_USAGE_RENDER, TransformParamsObj, *this);
    }

    virtual size_t GetHash() const override {
        size_t h = 0;
        HashCombine(h, std::hash<Vector3>{}(Translation));
        HashCombine(h, std::hash<Quaternion>{}(Rotation));
        HashCombine(h, std::hash<Vector3>{}(Scale));
        return h;
    }

    virtual size_t GetParamCount() override { return 3; }
    virtual std::string GetParamName(size_t index) override {
        static const std::string names[] = { "Translation", "Rotation", "Scale" };
        return index < GetParamCount() ? names[index] : "";
    }
    virtual std::any GetParamValue(const std::string& name) const override {
        if (name == "Translation") return Translation;
        if (name == "Rotation") return Rotation;
        if (name == "Scale") return Scale;
        return {};
    }

    virtual void Accept(ParamsVisitor& visitor) override {
        visitor.Visit(Translation, "Translation");
        visitor.Visit(Rotation, "Rotation");
        visitor.Visit(Scale, "Scale");
    }
    virtual void Accept(ConstParamsVisitor& visitor) const override {
        visitor.Visit(Translation, "Translation");
        visitor.Visit(Rotation, "Rotation");
        visitor.Visit(Scale, "Scale");
    }

    // ===== Type Info =====
    virtual std::string GetTypeName() const override { return "Transform"; }
    static const char* GetStaticTypeName() { return "Transform"; }
};

class TransformParams : public Handle<struct TransformParamsObj> {
public:
    explicit TransformParams(const Vector3& position)
        : Handle<struct TransformParamsObj>(
              CZ_NEW(MEMORY_USAGE_RENDER, TransformParamsObj, position)) {}

    TransformParams(const Vector3& position, const Quaternion& rotation, const Vector3& scale)
        : Handle<struct TransformParamsObj>(
              CZ_NEW(MEMORY_USAGE_RENDER, TransformParamsObj, position, rotation, scale)) {}

    TransformParams(float x, float y, float z) : TransformParams(Vector3(x, y, z)) {}

    using Handle<struct TransformParamsObj>::Handle;

    TransformParamsObj* Unwrap() { return InternalHandleReader::Unwrap(*this); }

    // ===== Type Info =====
    static const char* GetStaticTypeName() { return "TransformParams"; }

    // ===== Clone =====
    TransformParams Clone() const {
        if (!m_Obj) return TransformParams();
        return TransformParams(m_Obj->Clone());
    }
};

} // namespace CZ