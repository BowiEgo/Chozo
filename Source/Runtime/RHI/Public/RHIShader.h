#pragma once

#include "RHIExport.h"
#include "RHITypes.h"
#include "Ref.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRHIShader, Info);

using FShaderDefinitions = std::map<std::string, std::string>;

struct FRHIShaderSpecification {
    std::string Name;
    EShaderStage Stage;
    std::string EntryPoint = "main";

    FRHIShaderSpecification() = default;
    FRHIShaderSpecification(const std::string name, const EShaderStage stage,
                            const std::string entry)
        : Name(name), Stage(stage), EntryPoint(entry) {}
};

class RHI_API IRHIShader : public FRefCounted {
public:
    IRHIShader(const FRHIShaderSpecification& spec, const FShaderReflection reflection);
    virtual ~IRHIShader();

    const std::string GetName() const { return m_Spec.Name; }
    const EShaderStage GetStage() const { return m_Spec.Stage; }
    const std::string& GetEntryPoint() const { return m_Spec.EntryPoint; }
    const FShaderReflection GetReflection() const { return m_Reflection; }

    static TRef<IRHIShader> Create(const FRHIShaderSpecification& spec,
                                   const std::vector<uint32_t>* binary);

protected:
    FRHIShaderSpecification m_Spec;
    FShaderReflection m_Reflection;
};