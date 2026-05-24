#pragma once

#include <Core/Header/Handle.hpp>
#include <Runtime/RHI/RHITypes.hpp>

namespace CZ {

struct ShaderResSpecification {
    std::string Name;
    ShaderStage Stage;
    std::string EntryPoint;

    ShaderResSpecification() = default;
    ShaderResSpecification(const std::string name, const ShaderStage stage, const std::string entry)
        : Name(name), Stage(stage), EntryPoint(entry) {}
};

class ShaderResObj {
public:
    ShaderResObj(const ShaderResSpecification& spec) : m_Spec(spec) {}
    virtual ~ShaderResObj() = default;

    ShaderStage GetStage() const { return m_Spec.Stage; }

    const std::string& GetEntryPoint() const { return m_Spec.EntryPoint; }

protected:
    ShaderResSpecification m_Spec;
};

struct ShaderRes : Handle<class ShaderResObj> {
    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }

    template <typename T> const T* As() const {
        return static_cast<const T*>(InternalHandleReader::Unwrap(*this));
    }
};

} // namespace CZ
