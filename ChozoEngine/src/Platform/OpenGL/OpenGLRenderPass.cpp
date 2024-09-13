#include "OpenGLRenderPass.h"

#include "OpenGLUtils.h"
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
        m_UBs[std::string(name)] = uniformbuffer;
    }

    void OpenGLRenderPass::SetInput(std::string_view name, Ref<TextureCube> textureCube)
    {
    }


    Ref<Framebuffer> OpenGLRenderPass::GetTargetFramebuffer() const
    {
        return m_Specification.Pipeline->GetTargetFramebuffer();
    }

    void OpenGLRenderPass::Bake()
    {
        m_Specification.Pipeline->Render();
    }
}
