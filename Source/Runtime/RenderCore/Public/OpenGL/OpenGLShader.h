#include "Core.h"
#include "Shader.h"

DECLARE_LOG_CATEGORY_EXTERN(LogOpenGLShader, Info);

class RENDERCORE_API COpenGLShader : public CShader {
public:
    COpenGLShader(CGraphicsContext *context, const FShaderCreateInfo &rep,
                  const FShaderCompilerOutput &vsOutput,
                  const FShaderCompilerOutput &fsOutput);
    ~COpenGLShader() = default;

    void Bind() const override;
    void Unbind() const override;
    void ClearCache() override;
    void Compile() override;
    void AsyncCompile() override;

private:
};
