#pragma once

#include "Params.h"
#include "RHITypes.h"

inline const FParamControllerConfig kPolygonModeConfig{
    .Type  = EParamControllerType::Combo,
    .Items = std::vector<std::string>(FPolygonModeStrings.begin(), FPolygonModeStrings.end()),
    .bNotifyDirty = false
};
inline const FParamControllerConfig kCullModeConfig{
    .Type         = EParamControllerType::Combo,
    .Items        = std::vector<std::string>(FCullModeStrings.begin(), FCullModeStrings.end()),
    .bNotifyDirty = false
};

#define MATERIAL_PARAMS_LIST                                                                       \
    PARAM(EPolygonMode, PolygonMode, "Polygon Mode", kPolygonModeConfig)                           \
    PARAM(ECullMode, CullMode, "Cull Mode", kCullModeConfig)

struct RENDER_CORE_API IMaterialParams : public IParams {
    EPolygonMode PolygonMode = EPolygonMode::Fill;
    ECullMode CullMode       = ECullMode::Back;

    IMaterialParams() = default;
    IMaterialParams(const IMaterialParams& other)
        : IParams(), PolygonMode(other.PolygonMode), CullMode(other.CullMode) {}

    // ===== IParams Implementation =====
    virtual IParams* Clone() const override = 0;

    virtual bool Equals(const IParams& other) const override {
        const auto* otherMat = dynamic_cast<const IMaterialParams*>(&other);
        if (!otherMat) return false;

#define PARAM(type, member, ...)                                                                   \
    if (member != otherMat->member) return false;
        MATERIAL_PARAMS_LIST
#undef PARAM

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

    virtual void Accept(IParamsVisitor& visitor) override {
        visitor.VisitEnum(PolygonMode, "PolygonMode", kPolygonModeConfig);
        visitor.VisitEnum(CullMode, "CullMode", kCullModeConfig);

        Accept_Internal(visitor);
    }
    virtual void Accept(IConstParamsVisitor& visitor) const override {
        visitor.VisitEnum(PolygonMode, "PolygonMode", kPolygonModeConfig);
        visitor.VisitEnum(CullMode, "CullMode", kCullModeConfig);

        Accept_Internal(visitor);
    }

    virtual std::string GetTypeName() const override = 0;

    // ===== IMaterialParams Implementation =====
    virtual bool Equals_Internal(const IParams& other) const = 0;

    virtual void Accept_Internal(IParamsVisitor& visitor)            = 0;
    virtual void Accept_Internal(IConstParamsVisitor& visitor) const = 0;

    virtual const std::vector<std::string>& GetAllParamNames_Internal() = 0;

    const std::vector<std::string>& GetAllParamNames() {
        if (m_AllParamNames.empty()) {
            const auto& internal = GetAllParamNames_Internal();
            m_AllParamNames.reserve(internal.size() + 2);
            m_AllParamNames.insert(m_AllParamNames.end(), internal.begin(), internal.end());
#define PARAM(type, member, display, ...) m_AllParamNames.push_back(#member);
            MATERIAL_PARAMS_LIST
#undef PARAM
        }
        return m_AllParamNames;
    }

private:
    mutable std::vector<std::string> m_AllParamNames;
};
