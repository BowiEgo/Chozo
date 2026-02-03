#include "Core.h"
#include "Shader.h"

namespace Chozo {

DECLARE_LOG_CATEGORY_EXTERN(LogOpenGLShader, Info);

class RENDERCORE_API OpenGLShader : public Shader {
public:
    OpenGLShader(GraphicsContext *context, const ShaderCreateInfo &rep,
        const ShaderCompilerOutput &vsOutput,
        const ShaderCompilerOutput &fsOutput);
    ~OpenGLShader() = default;

    void Bind() const override;
    void Unbind() const override;
    void ClearCache() override;
    void Compile() override;
    void AsyncCompile() override;

private:
};
} // namespace Chozo
