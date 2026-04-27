#pragma once

#include "Params.h"
#include "RHITypes.h"

struct IMaterialParams : public IParams {
    EPolygonMode PolygonMode = EPolygonMode::Fill;
    ECullMode CullMode       = ECullMode::Back;

    IMaterialParams() = default;
    IMaterialParams(const IMaterialParams& other)
        : IParams(), PolygonMode(other.PolygonMode), CullMode(other.CullMode) {}

    // ===== IParams Implementation =====
    virtual IParams* Clone() const override = 0;

    virtual bool Equals(const IParams& other) const override = 0;

    virtual size_t GetHash() const override = 0;

    virtual std::string GetTypeName() const override = 0;

    virtual size_t GetPropertyCount() const override                     = 0;
    virtual std::string GetPropertyName(size_t index) const override     = 0;
    virtual std::any GetProperty(const std::string& name) const override = 0;

    virtual void Accept(IParamsVisitor& visitor) override            = 0;
    virtual void Accept(IConstParamsVisitor& visitor) const override = 0;
};