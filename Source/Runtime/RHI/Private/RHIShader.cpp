#include "RHIShader.h"

DEFINE_LOG_CATEGORY(LogRHIShader);

IRHIShader::IRHIShader(const FRHIShaderCreateInfo& info) : m_Info(info) {}

IRHIShader::~IRHIShader() {
    CZ_LOG(LogRHIShader, Trace, "RHIShader: {} destroying...", m_Info.Name);
}
