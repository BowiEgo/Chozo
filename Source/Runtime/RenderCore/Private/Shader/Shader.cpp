#include "Shader.h"

#include "RHIAPI.h"
#include "ShaderUtils.h"

DEFINE_LOG_CATEGORY(LogShader);

CShader::CShader(const FShaderSpecification& spec,
                 const std::unordered_map<EShaderStage, FShaderCompilerOutput>& outputs)
    : m_Spec(spec), m_Datas(outputs) {
    CZ_LOG(LogShader, Trace, "Creating shader {} ...", m_Spec.Name);
}

void CShader::CreateRHIDeviceResources() {
    m_ShaderResources.reserve(m_Datas.size());

    for (auto& [stage, output] : m_Datas) {
        FRHIShaderSpecification spec;
        spec.Name       = m_Spec.Name + "_" + ChozoUtils::Shader::StageToString(stage);
        spec.Stage      = stage;
        spec.EntryPoint = m_Spec.EntryPoint;

        auto RHIShader = IRHIAPI::CreateShader(spec, &output.Binary, output.Reflection);

        m_ShaderResources.push_back(RHIShader);
    }
}
