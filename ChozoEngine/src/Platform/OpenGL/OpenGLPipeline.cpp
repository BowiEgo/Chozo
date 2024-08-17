#include "OpenGLPipeline.h"

namespace Chozo
{

    OpenGLPipeline::OpenGLPipeline(PipelineSpecification &spec)
        : m_Shader(spec.Shader), m_TargetFramebuffer(spec.TargetFramebuffer)
    {
    }

    void OpenGLPipeline::Begin()
    {
        m_TargetFramebuffer->Bind();
    }

    void OpenGLPipeline::End()
    {
        m_TargetFramebuffer->Unbind();
    }
}
