#include "Shader.h"

#include "RHIAPI.h"
#include "ShaderUtils.h"

DEFINE_LOG_CATEGORY(LogShader);

CShader::CShader(const FShaderSpecification& spec, const FShaderCompilerOutput& data,
                 const WeakRef<IRHIDevice> device)
    : m_Spec(spec), m_Data(data), m_Device(device) {
    const std::string stageStr = ChozoUtils::Shader::StageToString(spec.Stage);

    m_Spec.Name = spec.Name + "_" + stageStr; // e.g., MyShader_Vertex

    CZ_LOG(LogShader, Trace, "Creating shader {} ...", m_Spec.Name);
}

TRef<IRHIShader> CShader::CreateRHIDeviceResource(const IRHIContext* ctx,
                                                  const FShaderCompilerOutput& data) {
    // Only create RHI resources for stages that were actually compiled
    auto device = m_Device.lock();
    if (!device) {
        CZ_LOG(LogShader, Error, "Device is no longer valid during shader creation!");
        return nullptr;
    }

    if (data.bSucceeded && !data.Binary.empty()) {
        // Use the Device to create the actual hardware resource
        auto RHIShader = IRHIAPI::CreateShader(ctx, m_Spec, &data.Binary);

        CZ_LOG(LogShader, Info, "RHI shader: {} created.", m_Spec.Name);

        return RHIShader;
    }

    return nullptr;
}
