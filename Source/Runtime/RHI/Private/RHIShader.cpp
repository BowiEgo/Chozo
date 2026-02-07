#include "RHIShader.h"

IRHIShader::IRHIShader(const FRHIShaderCreateInfo& info) : m_Info(info) {}

IRHIShader::~IRHIShader() = default;