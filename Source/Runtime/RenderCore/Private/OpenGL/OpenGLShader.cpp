#include "OpenGLShader.h"
#include "Logger.h"
#include "ShaderCompiler.h"

DEFINE_LOG_CATEGORY(LogOpenGLShader);

COpenGLShader::COpenGLShader(CGraphicsContext *context,
                             const FShaderCreateInfo &rep,
                             const FShaderCompilerOutput &vsOutput,
                             const FShaderCompilerOutput &fsOutput)
    : CShader(context, rep) {}
