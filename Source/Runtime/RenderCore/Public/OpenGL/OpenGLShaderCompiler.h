#pragma once

#include "ShaderCompiler.h"

class RENDERCORE_API COpenGLShaderCompiler : public CShaderCompiler {
public:
    COpenGLShaderCompiler() = default;
    ~COpenGLShaderCompiler() override = default;

    virtual bool CompileInternal(const FShaderCompilerInput &input,
                                 FShaderCompilerOutput &output) override;
};