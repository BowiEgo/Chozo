#pragma once

#include "ShaderCompiler.h"

namespace Chozo {

class RENDERCORE_API OpenGLShaderCompiler : public ShaderCompiler {
public:
    OpenGLShaderCompiler() = default;
    ~OpenGLShaderCompiler() override = default;

    virtual bool CompileInternal(const ShaderCompilerInput &input,
                                 ShaderCompilerOutput &output) override;
};
} // namespace Chozo