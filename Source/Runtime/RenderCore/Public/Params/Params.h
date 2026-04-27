#pragma once

#include "Ref.h"
#include "Scope.h"

#include "Asset.h"
#include "Quaternion.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

#include <any>
#include <string>

struct IParamsVisitor {
    virtual ~IParamsVisitor() = default;

    virtual void Visit(float& value, const std::string& name)       = 0;
    virtual void Visit(double& value, const std::string& name)      = 0;
    virtual void Visit(int32_t& value, const std::string& name)     = 0;
    virtual void Visit(uint32_t& value, const std::string& name)    = 0;
    virtual void Visit(int64_t& value, const std::string& name)     = 0;
    virtual void Visit(uint64_t& value, const std::string& name)    = 0;
    virtual void Visit(bool& value, const std::string& name)        = 0;
    virtual void Visit(std::string& value, const std::string& name) = 0;

    virtual void Visit(FVector2& value, const std::string& name)     = 0;
    virtual void Visit(FVector3& value, const std::string& name)     = 0;
    virtual void Visit(FVector4& value, const std::string& name)     = 0;
    virtual void Visit(FQuaternion& value, const std::string& name)  = 0;
    virtual void Visit(FAssetHandle& value, const std::string& name) = 0;
};

/**
 * Read Only
 */
struct IConstParamsVisitor {
    virtual ~IConstParamsVisitor() = default;

    virtual void Visit(const float& value, const std::string& name)        = 0;
    virtual void Visit(const double& value, const std::string& name)       = 0;
    virtual void Visit(const int32_t& value, const std::string& name)      = 0;
    virtual void Visit(const uint32_t& value, const std::string& name)     = 0;
    virtual void Visit(const int64_t& value, const std::string& name)      = 0;
    virtual void Visit(const uint64_t& value, const std::string& name)     = 0;
    virtual void Visit(const bool& value, const std::string& name)         = 0;
    virtual void Visit(const std::string& value, const std::string& name)  = 0;
    virtual void Visit(const FVector2& value, const std::string& name)     = 0;
    virtual void Visit(const FVector3& value, const std::string& name)     = 0;
    virtual void Visit(const FVector4& value, const std::string& name)     = 0;
    virtual void Visit(const FQuaternion& value, const std::string& name)  = 0;
    virtual void Visit(const FAssetHandle& value, const std::string& name) = 0;
};

/**
 * Abstract parameter interface
 * Inherits from FRefCounted for shared ownership
 */
struct IParams : public FRefCounted {
    virtual ~IParams()                              = default;
    virtual IParams* Clone() const                  = 0;
    virtual bool Equals(const IParams& other) const = 0;
    virtual size_t GetHash() const                  = 0;
    virtual std::string GetTypeName() const         = 0;

    virtual size_t GetPropertyCount() const                 = 0;
    virtual std::string GetPropertyName(size_t index) const = 0;

    virtual std::any GetProperty(const std::string& name) const = 0;

    virtual void Accept(IParamsVisitor& visitor)            = 0;
    virtual void Accept(IConstParamsVisitor& visitor) const = 0;
};

struct IParamsFactory {
    virtual ~IParamsFactory()                                  = default;
    virtual TScope<IParams> CreateDefault() const              = 0;
    virtual TScope<IParams> Clone(const IParams* params) const = 0;
    virtual std::string GetTypeName() const                    = 0;
};

template <typename T> struct TParamsFactory : public IParamsFactory {
    TScope<IParams> CreateDefault() const override { return CreateScope<T>(); }

    TScope<IParams> Clone(const IParams* params) const override {
        const auto* typed = dynamic_cast<const T*>(params);
        if (typed) {
            return CreateScope<T>(*typed);
        }
        return nullptr;
    }

    std::string GetTypeName() const override { return T::GetStaticTypeName(); }
};