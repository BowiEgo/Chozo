#include "Shader.h"

#include "ShaderUtils.h"

DEFINE_LOG_CATEGORY(LogShader);

CShader::CShader(const FShaderCreateInfo& info, const FShaderCompilerOutput& data,
                 const WeakRef<IRHIDevice> device)
    : m_Info(info), m_Data(data), m_Device(device) {
    CZ_LOG(LogShader, Trace, "Creating shader", info.Name);
}

TRef<IRHIShader> CShader::CreateRHIDeviceResource(const FShaderCreateInfo& info,
                                                  const FShaderCompilerOutput& data) {
    // Only create RHI resources for stages that were actually compiled
    if (data.bSucceeded && !data.Binary.empty()) {
        const std::string stageStr = ChozoUtils::Shader::StageToString(info.Stage);

        FRHIShaderCreateInfo createInfo;
        createInfo.Stage = info.Stage;
        createInfo.Name = info.Name + "_" + stageStr; // e.g., MyShader_Vertex
        createInfo.EntryPoint = info.EntryPoint;

        // Use the Device to create the actual hardware resource
        auto RHIShader = m_Device->CreateShader(createInfo, &data.Binary);

        CZ_LOG(LogShader, Info, "RHI shader: {} created", createInfo.Name, stageStr);

        return RHIShader;
    }

    return nullptr;
}
