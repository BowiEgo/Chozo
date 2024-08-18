#include "OpenGLPipeline.h"

namespace Chozo
{

    OpenGLPipeline::OpenGLPipeline(PipelineSpecification &spec)
        : m_Shader(spec.Shader), m_TargetFramebuffer(spec.TargetFramebuffer), m_DynamicMesh(spec.DynamicMesh)
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

    void OpenGLPipeline::Render()
    {
        Begin();
        m_RenderFunction();
        End();
    }
}
