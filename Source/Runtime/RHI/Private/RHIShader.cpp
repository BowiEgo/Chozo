#include "RHIShader.h"

IRHIShader::IRHIShader(const FRHIShaderCreateInfo& info)
    : m_Stage(info.Stage), m_Name(info.Name) {}

IRHIShader::~IRHIShader() = default;