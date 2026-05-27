#pragma once

#include "Core/Memory/Memory.hpp"
#include "Core/Memory/MemoryTypes.hpp"
#include <Core/Math/Quaternion.hpp>
#include <Core/Math/Vector2.hpp>
#include <Core/Math/Vector3.hpp>
#include <Core/Math/Vector4.hpp>
#include <Runtime/RenderCore/Asset.hpp>

#include <any>
#include <cfloat>
#include <string>

namespace CZ {

enum class ParamControllerType {
    Default,
    Drag,
    Slider,
    ColorPicker,
    AssetPicker,
    Combo,
    Checkbox,
    Radio,
};

struct ParamControllerConfig {
    ParamControllerType Type = ParamControllerType::Default;
    float Min                = -FLT_MAX;
    float Max                = FLT_MAX;
    float Speed              = 0.01f;
    std::vector<std::string> Items; // Use for Combo
    bool bNotifyDirty = true;
};

struct ParamMeta {
    std::string Name;
    std::string DisplayName;
    ParamControllerConfig Config;
};

struct ParamsVisitor {
    virtual ~ParamsVisitor() = default;

    virtual void Visit(float& value, const std::string& name,
                       const ParamControllerConfig config = {}) = 0;
    virtual void Visit(double& value, const std::string& name,
                       const ParamControllerConfig config = {}) = 0;
    virtual void Visit(int32_t& value, const std::string& name,
                       const ParamControllerConfig config = {}) = 0;
    virtual void Visit(uint32_t& value, const std::string& name,
                       const ParamControllerConfig config = {}) = 0;
    virtual void Visit(int64_t& value, const std::string& name,
                       const ParamControllerConfig config = {}) = 0;
    virtual void Visit(uint64_t& value, const std::string& name,
                       const ParamControllerConfig config = {}) = 0;
    virtual void Visit(bool& value, const std::string& name,
                       const ParamControllerConfig config = {}) = 0;
    virtual void Visit(std::string& value, const std::string& name,
                       const ParamControllerConfig config = {}) = 0;

    virtual void Visit(Vector2& value, const std::string& name,
                       const ParamControllerConfig config = {}) = 0;
    virtual void Visit(Vector3& value, const std::string& name,
                       const ParamControllerConfig config = {}) = 0;
    virtual void Visit(Vector4& value, const std::string& name,
                       const ParamControllerConfig config = {}) = 0;
    virtual void Visit(Quaternion& value, const std::string& name,
                       const ParamControllerConfig config = {}) = 0;
    virtual void Visit(AssetHandle& value, const std::string& name,
                       const ParamControllerConfig config = {}) = 0;

    template <typename T>
    void VisitEnum(T& value, const std::string& name, const ParamControllerConfig config = {}) {
        if constexpr (std::is_enum_v<T>) {
            uint32_t intVal = static_cast<uint32_t>(value);
            Visit(intVal, name, config);
            value = static_cast<T>(intVal);
        }
    }
};

/**
 * Read Only
 */
struct ConstParamsVisitor {
    virtual ~ConstParamsVisitor() = default;

    virtual void Visit(const float& value, const std::string& name,
                       const ParamControllerConfig config = {}) const = 0;
    virtual void Visit(const double& value, const std::string& name,
                       const ParamControllerConfig config = {}) const = 0;
    virtual void Visit(const int32_t& value, const std::string& name,
                       const ParamControllerConfig config = {}) const = 0;
    virtual void Visit(const uint32_t& value, const std::string& name,
                       const ParamControllerConfig config = {}) const = 0;
    virtual void Visit(const int64_t& value, const std::string& name,
                       const ParamControllerConfig config = {}) const = 0;
    virtual void Visit(const uint64_t& value, const std::string& name,
                       const ParamControllerConfig config = {}) const = 0;
    virtual void Visit(const bool& value, const std::string& name,
                       const ParamControllerConfig config = {}) const = 0;
    virtual void Visit(const std::string& value, const std::string& name,
                       const ParamControllerConfig config = {}) const = 0;
    virtual void Visit(const Vector2& value, const std::string& name,
                       const ParamControllerConfig config = {}) const = 0;
    virtual void Visit(const Vector3& value, const std::string& name,
                       const ParamControllerConfig config = {}) const = 0;
    virtual void Visit(const Vector4& value, const std::string& name,
                       const ParamControllerConfig config = {}) const = 0;
    virtual void Visit(const Quaternion& value, const std::string& name,
                       const ParamControllerConfig config = {}) const = 0;
    virtual void Visit(const AssetHandle& value, const std::string& name,
                       const ParamControllerConfig config = {}) const = 0;

    template <typename T>
    void VisitEnum(T& value, const std::string& name,
                   const ParamControllerConfig config = {}) const {
        if constexpr (std::is_enum_v<T>) {
            uint32_t intVal = static_cast<uint32_t>(value);
            Visit(intVal, name, config);
        }
    }
};

/**
 * Abstract parameter interface
 */
struct Params {
    virtual ~Params()                              = default;
    virtual Params* Clone() const                  = 0;
    virtual bool Equals(const Params& other) const = 0;
    virtual size_t GetHash() const                 = 0;

    virtual size_t GetParamCount()                 = 0;
    virtual std::string GetParamName(size_t index) = 0;

    virtual std::any GetParamValue(const std::string& name) const = 0;

    virtual void Accept(ParamsVisitor& visitor)            = 0;
    virtual void Accept(ConstParamsVisitor& visitor) const = 0;

    virtual std::string GetTypeName() const = 0;
};

struct ParamsFactory {
    virtual ~ParamsFactory()                                = default;
    virtual Scope<Params> CreateDefault() const             = 0;
    virtual Scope<Params> Clone(const Params* params) const = 0;
    virtual std::string GetTypeName() const                 = 0;
};

template <typename T> struct TParamsFactory : public ParamsFactory {
    Scope<Params> CreateDefault() const override { return CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, T); }

    Scope<Params> Clone(const Params* params) const override {
        const auto* typed = dynamic_cast<const T*>(params);
        if (typed) {
            return CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, T, *typed);
        }
        return nullptr;
    }

    std::string GetTypeName() const override { return T::GetStaticTypeName(); }
};

} // namespace CZ
