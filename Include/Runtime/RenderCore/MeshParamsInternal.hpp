#pragma once

#include <Runtime/RenderCore/Params.hpp>

namespace CZ {

struct MeshParamsInternal : public Params {
    AssetHandle Material;

    MeshParamsInternal() = default;
    MeshParamsInternal(const MeshParamsInternal& other) : Params(), Material(other.Material) {}

    // ===== Params Implementation =====
    virtual Params* Clone() const override = 0;

    virtual bool Equals(const Params& other) const override {
        const auto* otherMat = dynamic_cast<const MeshParamsInternal*>(&other);
        if (!otherMat) return false;

        if (Material != otherMat->Material) return false;

        if (!Equals_Internal(other)) return false;

        return true;
    }

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
    virtual bool Equals_Internal(const Params& other) const = 0;

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

} // namespace CZ
