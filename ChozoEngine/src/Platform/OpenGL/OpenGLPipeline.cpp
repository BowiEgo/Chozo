#include "OpenGLPipeline.h"

#include "OpenGLShader.h"

namespace Chozo
{

    OpenGLPipeline::OpenGLPipeline(PipelineSpecification &spec)
        : m_Shader(spec.Shader), m_TargetFramebuffer(spec.TargetFramebuffer), m_Spec(spec)
    {
    }

    void OpenGLPipeline::BindUniformBlock()
    {
        if (!m_Shader)
            return;

        for (auto [name, uniformBuffer] : m_UBs)
            m_Shader.As<OpenGLShader>()->SetUniformBlockBinding(std::string(name), uniformBuffer.As<OpenGLUniformBuffer>()->GetBindingPoint());
    }
}
