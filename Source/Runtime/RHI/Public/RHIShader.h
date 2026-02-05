#pragma once

#include "RHIDevice.h"
#include "RHIExport.h"
#include "RHITypes.h"
#include "Ref.h"

struct FRHIShaderCreateInfo {
    TRef<IRHIDevice> Device;
    EShaderStage Stage;
    std::string Name;
    const std::vector<uint32_t>* Binary = nullptr;
};

class RHI_API IRHIShader : public FRefCounted {
public:
    IRHIShader(const FRHIShaderCreateInfo& info)
        : m_Stage(info.Stage), m_Name(info.Name), m_Device(info.Device) {}

    virtual ~IRHIShader() = default;

    const EShaderStage GetStage() const { return m_Stage; };
    const std::string& GetName() const { return m_Name; };

    static TRef<IRHIShader> Create(const FRHIShaderCreateInfo& info);

protected:
    EShaderStage m_Stage;
    std::string m_Name;
    TRef<IRHIDevice> m_Device;
};