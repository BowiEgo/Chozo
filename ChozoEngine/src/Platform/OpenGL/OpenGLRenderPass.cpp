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

    void OpenGLRenderPass::SetOutput(std::string_view name, Ref<Texture2D> texture)
    {
        m_Outputs[std::string(name)] = texture;
    }

    void OpenGLRenderPass::SetOutput(std::string_view name, Ref<TextureCube> textureCube)
    {
        m_Outputs[std::string(name)] = textureCube;
    }

    Ref<Texture> OpenGLRenderPass::GetOutput(std::string_view name)
    {
        return m_Outputs[std::string(name)];
    }

    Ref<Framebuffer> OpenGLRenderPass::GetTargetFramebuffer() const
    {
        return m_Specification.Pipeline->GetTargetFramebuffer();
    }

    void OpenGLRenderPass::Bake()
    {
        for (const auto& [name, output] : m_Outputs)
        {
            m_Specification.Pipeline->Render();
        }
    }
}
