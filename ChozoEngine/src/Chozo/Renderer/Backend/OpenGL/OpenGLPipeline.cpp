#include "OpenGLPipeline.h"

#include "OpenGLShader.h"

namespace Chozo
{

    OpenGLPipeline::OpenGLPipeline(PipelineSpecification &spec)
        : m_Spec(spec)
    {
    }

    void OpenGLPipeline::BindUniformBlock()
    {
        if (!m_Spec.Shader)
            return;

        for (auto [name, uniformBuffer] : m_UBs)
            m_Spec.Shader.As<OpenGLShader>()->SetUniformBlockBinding(std::string(name), uniformBuffer.As<OpenGLUniformBuffer>()->GetBindingPoint());
    }
}
