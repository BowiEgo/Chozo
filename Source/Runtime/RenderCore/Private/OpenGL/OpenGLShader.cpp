#include "OpenGLShader.h"
#include "Logger.h"
#include "ShaderCompiler.h"

namespace Chozo {

DEFINE_LOG_CATEGORY(LogOpenGLShader);

OpenGLShader::OpenGLShader(GraphicsContext *context,
    const ShaderCreateInfo &rep, const ShaderCompilerOutput &vsOutput,
    const ShaderCompilerOutput &fsOutput)
    : Shader(context, rep) {}

void OpenGLShader::Bind() const {}
void OpenGLShader::Unbind() const {}
void OpenGLShader::ClearCache() {}
void OpenGLShader::Compile() {
    CZ_LOG(LogOpenGLShader, Trace, "Compiling shader '{}'", m_Rep.Name);
}
void OpenGLShader::AsyncCompile() {}
} // namespace Chozo
