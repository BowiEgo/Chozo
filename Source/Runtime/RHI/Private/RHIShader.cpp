#include "RHIShader.h"

IRHIShader::IRHIShader(const FRHIShaderCreateInfo& info) : m_Data(info) {}

IRHIShader::~IRHIShader() = default;