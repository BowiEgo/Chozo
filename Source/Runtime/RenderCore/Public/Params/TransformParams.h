#pragma once

#include "RenderCoreExport.h"

#include "MathUtils.h"
#include "Matrix4.h"
#include "Params.h"
#include "Quaternion.h"
#include "Vector3.h"

struct RENDER_CORE_API FTransformParams : public IParams {
    FVector3 Translation = FVector3::Zero;
    FQuaternion Rotation = FQuaternion::Identity();
    FVector3 Scale       = FVector3::One;

    FTransformParams() = default;

    FTransformParams(FVector3 translation, FQuaternion rotation, FVector3 scale)
        : Translation(translation), Rotation(rotation), Scale(scale) {}

    FTransformParams(const FTransformParams& other)
        : Translation(other.Translation), Rotation(other.Rotation), Scale(other.Scale) {}

    FTransformParams& operator=(const FTransformParams& other) {
        if (this != &other) {
            Translation = other.Translation;
            Rotation    = other.Rotation;
            Scale       = other.Scale;
        }
        return *this;
    }

    // ===== IParams Implementation =====
    virtual IParams* Clone() const override { return new FTransformParams(*this); }

    virtual bool Equals(const IParams& other) const override {
        const auto* otherCube = dynamic_cast<const FTransformParams*>(&other);
        if (!otherCube) return false;

        return Translation == otherCube->Translation && Rotation == otherCube->Rotation &&
               Scale == otherCube->Scale;
    }

    virtual size_t GetHash() const override {
        size_t h = 0;
        HashCombine(h, std::hash<FVector3>{}(Translation));
        HashCombine(h, std::hash<FQuaternion>{}(Rotation));
        HashCombine(h, std::hash<FVector3>{}(Scale));
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

    virtual void Accept(IParamsVisitor& visitor) override {
        visitor.Visit(Translation, "Translation");
        visitor.Visit(Rotation, "Rotation");
        visitor.Visit(Scale, "Scale");
    }
    virtual void Accept(IConstParamsVisitor& visitor) const override {
        visitor.Visit(Translation, "Translation");
        visitor.Visit(Rotation, "Rotation");
        visitor.Visit(Scale, "Scale");
    }

    // ===== Type Info =====
    virtual std::string GetTypeName() const override { return "Transform"; }
    static const char* GetStaticTypeName() { return "Transform"; }

    // ===== Comparison Operators =====
    bool operator==(const FTransformParams& other) const { return Equals(other); }
    bool operator!=(const FTransformParams& other) const { return !(*this == other); }
};
