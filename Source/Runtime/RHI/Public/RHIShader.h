#pragma once

#include "RHIDevice.h"
#include "RHIExport.h"
#include "RHITypes.h"
#include "Ref.h"

struct FRHIShaderCreateInfo {
    EShaderStage Stage;
    std::string Name;
};

class RHI_API IRHIShader : public FRefCounted {
public:
    IRHIShader(const FRHIShaderCreateInfo& info);
    virtual ~IRHIShader();

    const EShaderStage GetStage() const { return m_Data.Stage; };
    const std::string& GetName() const { return m_Data.Name; };

    static TRef<IRHIShader> Create(const FRHIShaderCreateInfo& info,
                                   const std::vector<uint32_t>* binary);

protected:
    FRHIShaderCreateInfo m_Data;
};