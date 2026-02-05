#include "Shader.h"

#include "ShaderUtils.h"

DEFINE_LOG_CATEGORY(LogShader);

CShader::CShader(const TRef<IRHIDevice> device, const FShaderCompiledData &data)
    : m_Device(device), m_ID(data.ID), m_Name(data.Name) {
    CZ_LOG(LogShader, Trace, "Creating shader", data.Name);
    CreateRHIDeviceResource(data);
}

void CShader::CreateRHIDeviceResource(const FShaderCompiledData &data) {
    for (uint32 i = 0; i < kShaderStageCount; ++i) {
        const FShaderCompilerOutput &output = data.StageOutputs[i];

        // [Note] Only create RHI resources for stages that were actually
        // compiled
        if (output.bSucceeded && !output.Binary.empty()) {
            EShaderStage stage = static_cast<EShaderStage>(i);

            FRHIShaderCreateInfo createInfo;
            createInfo.Device =
                m_Device; // [Note] Ensure the RHI knows which device to use
            createInfo.Stage = stage;
            createInfo.Name =
                m_Name + "_" +
                ChozoUtils::Shader::StageToString(stage); // e.g., MyShader_0
            createInfo.Binary = &output.Binary;

            // [Note] Use the Device to create the actual hardware resource
            m_RHIShaders[static_cast<size_t>(stage)] =
                m_Device->CreateShader(createInfo);

            CZ_LOG(LogShader, Trace, "Created RHI shader stage: {0}", i);
        }
    }
}
