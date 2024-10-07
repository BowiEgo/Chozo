#include "OpenGLRenderPass.h"

#include "OpenGLPipeline.h"
#include "Chozo/Renderer/RenderCommand.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Chozo {

    OpenGLRenderPass::OpenGLRenderPass(const RenderPassSpecification& spec)
        : m_Specification(spec)
    {
    }

    void OpenGLRenderPass::SetInput(std::string_view name, Ref<UniformBuffer> uniformbuffer)
    {
        m_Specification.Pipeline.As<OpenGLPipeline>()->m_UBs[std::string(name)] = uniformbuffer.As<OpenGLUniformBuffer>();
    }

    void OpenGLRenderPass::SetInput(std::string_view name, Ref<TextureCube> textureCube)
    {
    }

    Ref<Texture2D> OpenGLRenderPass::GetOutput(uint32_t index)
    {
        Ref<Framebuffer> framebuffer = GetTargetFramebuffer();
        return framebuffer->GetImage(index);
    }

    Ref<Pipeline> OpenGLRenderPass::GetPipeline() const
    {
        return m_Specification.Pipeline;
    }

    Ref<Framebuffer> OpenGLRenderPass::GetTargetFramebuffer() const
    {
        return m_Specification.Pipeline->GetTargetFramebuffer();
    }

    void OpenGLRenderPass::Bake()
    {
    }
}
