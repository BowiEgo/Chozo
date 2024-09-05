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

    void OpenGLRenderPass::SetInput(std::string_view name, Ref<TextureCube> textureCube)
    {
    }

    void OpenGLRenderPass::SetOutput(std::string_view name, Ref<TextureCube> textureCube)
    {
        m_Outputs[std::string(name)] = textureCube;
    }

    Ref<TextureCube> OpenGLRenderPass::GetOutput(std::string_view name)
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
            if (dynamic_cast<TextureCube*>(output.get()))
            {
                static const glm::mat4 captureViews[] = 
                {
                    glm::lookAt(glm::vec3(0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                    glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                    glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
                    glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
                    glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                    glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
                };
                static const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

                output->Bind();
                GetTargetFramebuffer()->Bind();
                for (uint32_t i = 0; i < 6; i++)
                {
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, output->GetRendererID(), 0); GCE;
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GCE;

                    m_Specification.Pipeline->GetShader()->Bind();
                    m_Specification.Pipeline->GetShader()->SetUniform("u_Camera.ViewMatrix", captureViews[i]);
                    m_Specification.Pipeline->GetShader()->SetUniform("u_Camera.ProjectionMatrix", captureProjection);

                    RenderCommand::DrawIndexed(m_Specification.Pipeline->GetDynamicMesh()->GetVertexArray(), 0);
                }
                GetTargetFramebuffer()->Unbind();
            }
        }
    }
}
