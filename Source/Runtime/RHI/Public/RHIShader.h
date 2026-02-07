#pragma once

#include "RHIExport.h"
#include "RHITypes.h"
#include "Ref.h"

struct FRHIShaderCreateInfo {
    EShaderStage Stage;
    std::string Name;
    std::string EntryPoint = "main";
};

class RHI_API IRHIShader : public FRefCounted {
public:
    IRHIShader(const FRHIShaderCreateInfo& info);
    virtual ~IRHIShader();

    const EShaderStage GetStage() const { return m_Info.Stage; };
    const std::string& GetName() const { return m_Info.Name; };
    const std::string& GetEntryPoint() const { return m_Info.EntryPoint; };

    static TRef<IRHIShader> Create(const FRHIShaderCreateInfo& info,
                                   const std::vector<uint32_t>* binary);

protected:
    FRHIShaderCreateInfo m_Info;
};