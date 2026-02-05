#include "Core.h"
#include "Shader.h"

DECLARE_LOG_CATEGORY_EXTERN(LogOpenGLShader, Info);

class RENDER_CORE_API COpenGLShader : public CShader {
public:
    COpenGLShader(CGraphicsContext *context, const FShaderCreateInfo &rep,
                  const FShaderCompilerOutput &vsOutput,
                  const FShaderCompilerOutput &fsOutput);
    ~COpenGLShader() = default;

private:
};
