#include "Shader.h"

#include "ShaderUtils.h"

DEFINE_LOG_CATEGORY(LogShader);

CShader::CShader(const FShaderCompiledData& data)
    : m_ID(data.ID), m_Name(data.Name) {
    CZ_LOG(LogShader, Trace, "Creating shader", data.Name);
    CreateRHIDeviceResource(data);
}

void CShader::CreateRHIDeviceResource(const FShaderCompiledData& data) {
    const auto device = CGraphicsContext::Get().GetRHI()->GetDevice();
    CZ_CORE_ASSERT(device, "No RHIDevice of GraphicsContext exist");

    for (uint32 i = 0; i < kShaderStageCount; ++i) {
        const FShaderCompilerOutput& output = data.StageOutputs[i];

        // Only create RHI resources for stages that were actually compiled
        if (output.bSucceeded && !output.Binary.empty()) {
            EShaderStage stage = static_cast<EShaderStage>(i);

            FRHIShaderCreateInfo createInfo;
            createInfo.Stage = stage;
            createInfo.Name = m_Name + "_" +
                              ChozoUtils::Shader::StageToString(
                                  stage); // e.g., MyShader_Vertex

            // Use the Device to create the actual hardware resource
            m_RHIShaders[static_cast<size_t>(stage)] =
                device->CreateShader(createInfo, &output.Binary);

            CZ_LOG(LogShader, Trace, "Created RHI shader stage: {0}", i);
        }
    }
}
