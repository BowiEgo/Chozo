#include "OpenGLShader.h"
#include "Logger.h"
#include "ShaderCompiler.h"

DEFINE_LOG_CATEGORY(LogOpenGLShader);

COpenGLShader::COpenGLShader(CGraphicsContext *context,
                             const FShaderCreateInfo &rep,
                             const FShaderCompilerOutput &vsOutput,
                             const FShaderCompilerOutput &fsOutput)
    : CShader(context, rep) {}

void COpenGLShader::Bind() const {}
void COpenGLShader::Unbind() const {}
void COpenGLShader::ClearCache() {}
void COpenGLShader::Compile() {
    CZ_LOG(LogOpenGLShader, Trace, "Compiling shader '{}'", m_Rep.Name);
}
void COpenGLShader::AsyncCompile() {}
