#include "RHIShader.h"

DEFINE_LOG_CATEGORY(LogRHIShader);

IRHIShader::IRHIShader(const FShaderSpecification& spec, const FShaderReflection reflection)
    : m_Spec(spec), m_Reflection(reflection) {}

IRHIShader::~IRHIShader() {
    CZ_LOG(LogRHIShader, Trace, "RHIShader: {} destroying...", m_Spec.Name);
}
