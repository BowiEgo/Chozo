#include "RHIShader.h"

DEFINE_LOG_CATEGORY(LogRHIShader);

IRHIShader::IRHIShader(const FShaderSpecification& spec) : m_Spec(spec) {}

IRHIShader::~IRHIShader() {
    CZ_LOG(LogRHIShader, Trace, "RHIShader: {} destroying...", m_Spec.Name);
}
