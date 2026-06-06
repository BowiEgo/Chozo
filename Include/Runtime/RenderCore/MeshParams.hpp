#pragma once

#include <Core/Math/Matrix4.hpp>
#include <Core/Math/Vector2.hpp>
#include <Core/Math/Vector3.hpp>
#include <Core/Math/Vector4.hpp>
#include <Runtime/RenderCore/Params.hpp>

namespace CZ {

struct MeshParamsObj : public Params {
    AssetHandle Material;

    MeshParamsObj() = default;
    MeshParamsObj(const MeshParamsObj& other) : Params(), Material(other.Material) {}

    bool operator==(const MeshParamsObj* other) const {
        if (!other) return false;

        if (Material != other->Material) return false;

        if (!Equals_Internal(*other)) return false;

        return true;
    }

    // ===== Params Implementation =====
    virtual MeshParamsObj* Clone() const override = 0;

    virtual size_t GetHash() const override = 0;

    virtual size_t GetParamCount() override { return GetAllParamNames().size(); }
    virtual std::string GetParamName(size_t index) override {
        const auto& names = GetAllParamNames();
        if (index < names.size()) return names[index];
        return "";
    }
    virtual std::any GetParamValue(const std::string& name) const override = 0;

    virtual void Accept(ParamsVisitor& visitor) override {
        Accept_Internal(visitor);
        visitor.Visit(Material, "Material");
    }
    virtual void Accept(ConstParamsVisitor& visitor) const override {
        Accept_Internal(visitor);
        visitor.Visit(Material, "Material");
    }

    virtual std::string GetTypeName() const override = 0;

    // ===== MeshParamsInternal Implementation =====
    virtual bool Equals_Internal(MeshParamsObj& other)              = 0;
    virtual bool Equals_Internal(const MeshParamsObj& other) const  = 0;
    virtual void Accept_Internal(ParamsVisitor& visitor)            = 0;
    virtual void Accept_Internal(ConstParamsVisitor& visitor) const = 0;

    virtual const std::vector<std::string>& GetAllParamNames_Internal() = 0;

    const std::vector<std::string>& GetAllParamNames() {
        if (m_AllParamNames.empty()) {
            const auto& internal = GetAllParamNames_Internal();
            m_AllParamNames.reserve(internal.size() + 2);
            m_AllParamNames.insert(m_AllParamNames.end(), internal.begin(), internal.end());

            m_AllParamNames.push_back("Material");
        }
        return m_AllParamNames;
    }

private:
    mutable std::vector<std::string> m_AllParamNames;
};

/**
 * MeshParams - Type-erased wrapper for mesh parameters with value semantics
 *
 * Features:
 * - Shallow copy (shared IParams) via default copy operations
 * - Deep copy via Clone() method
 * - Type-safe access via As<T>()
 * - Automatic reference counting via TRef
 * - Equality comparison based on parameter content
 *
 * Example:
 * @code
 * MeshParams sphere(SphereParams(1.0f, 32, 16));
 * MeshParams sphere2 = sphere;                    // Shallow copy
 * if (auto* p = sphere.As<SphereParams>()) {
 *     p->Radius = 2.0f;                            // Modifies both sphere and sphere2
 * }
 * MeshParams sphere3 = sphere.Clone();            // Deep copy
 * @endcode
 */
class MeshParams : public Handle<struct MeshParamsObj> {
public:
    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }

    // ===== Type Info =====
    static const char* GetStaticTypeName() { return "MeshParams"; }

    // ===== Clone =====
    MeshParams Clone() const {
        if (!m_Obj) return MeshParams();
        return MeshParams(m_Obj->Clone());
    }
};

} // namespace CZ
