#include "OpenGLRendererAPI.h"

#include "OpenGLUtils.h"

#include "Chozo/Renderer/Renderer.h"
#include "Chozo/Renderer/Mesh.h"
// #include "OpenGLFrameBuffer.h"
#include "OpenGLRenderPass.h"
#include "OpenGLMaterial.h"

#include <glad/glad.h>

namespace Chozo {

    static const glm::mat4 CubeTextureCaptureViews[] = 
    {
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    static const glm::mat4 CubeTextureCaptureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

    void OpenGLRendererAPI::Init()
    {
        glEnable(GL_BLEND); GCE;
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); GCE;

        glEnable(GL_DEPTH_TEST); GCE;
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); GCE;
    }

    int OpenGLRendererAPI::GetMaxTextureSlots()
    {
        int maxTextureImageUnits;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureImageUnits); GCE;
        return maxTextureImageUnits;
    }

    void Chozo::OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
    {
        glClearColor(color.r, color.g, color.b, color.b); GCE;
    }

    void OpenGLRendererAPI::Clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GCE;
    }

    void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
    {
        uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
        vertexArray->Bind();
        vertexArray->GetIndexBuffer()->Bind();
        for(Ref<VertexBuffer> vertexBuffer : vertexArray->GetVertexBuffers())
            vertexBuffer->Bind();
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr); GCE;
    }

    void OpenGLRendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
    {
        vertexArray->Bind();

        glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCount); GCE;
    }

    void OpenGLRendererAPI::DrawPreethamSky(const float turbidity, const float azimuth, const float inclination)
    {
        Ref<Shader> preethamSkyShader = Renderer::GetRendererData().m_PreethamSkyRenderPass->GetSpecification().Pipeline->GetShader();
        preethamSkyShader->Bind();
        preethamSkyShader->SetUniform("u_FragUniforms.Turbidity", turbidity);
        preethamSkyShader->SetUniform("u_FragUniforms.Azimuth", azimuth);
        preethamSkyShader->SetUniform("u_FragUniforms.Inclination", inclination);

        Renderer::GetRendererData().m_PreethamSkyRenderPass->Bake();
    }

    void OpenGLRendererAPI::DrawEnvMap(const Ref<Shader> &shader, const Ref<TextureCube> &textureCube, const Ref<VertexArray> &VAO)
    {
        textureCube->Bind();
        for (uint32_t i = 0; i < 6; i++)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, textureCube->GetRendererID(), 0); GCE;
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GCE;

            shader->Bind();
            shader->SetUniform("u_Camera.ViewMatrix", CubeTextureCaptureViews[i]);
            shader->SetUniform("u_Camera.ProjectionMatrix", CubeTextureCaptureProjection);

            RenderCommand::DrawIndexed(VAO, 0);
        }
    }

    void OpenGLRendererAPI::DrawSkyLight(const Ref<Environment>& environment, const float& environmentIntensity, const float& skyboxLod, const EditorCamera& camera)
    {
        environment->IrradianceMap->Bind();
        Ref<Shader> shader = Renderer::GetRendererData().m_ShaderLibrary->Get("Skybox");
        shader->Bind();

        shader->SetUniform("u_Texture", 0);
        shader->SetUniform("u_FragUniforms.Intensity", environmentIntensity);
        shader->SetUniform("u_FragUniforms.TextureLod", skyboxLod);

        // TODO: Change to pipeline context status.
        glDepthFunc(GL_LEQUAL); GCE;
        RenderCommand::DrawIndexed(Renderer::GetRendererData().BoxMesh->GetVertexArray(), 0);
        glDepthFunc(GL_LESS); GCE;
    }

    void OpenGLRendererAPI::BeginRenderPass(Ref<RenderCommandBuffer> commandBuffer, Ref<RenderPass> renderPass)
    {
        commandBuffer->AddCommand([renderPass]()
        {
            renderPass->GetTargetFramebuffer()->Bind();
        });
    }

    void OpenGLRendererAPI::EndRenderPass(Ref<RenderCommandBuffer> commandBuffer, Ref<RenderPass> renderPass)
    {
        commandBuffer->AddCommand([renderPass]()
        {
            renderPass->GetTargetFramebuffer()->Unbind();
        });
    }

    void OpenGLRendererAPI::SubmitFullscreenBox(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<Material> material)
    {
        commandBuffer->AddCommand([pipeline, material]()
        {
            OpenGLShader* shader = dynamic_cast<OpenGLShader*>(pipeline->GetShader().get());
            dynamic_cast<OpenGLMaterial*>(material.get())->BindTextures();

            shader->Bind();
            // shader->SetUniformBlockBinding();

            glDepthFunc(GL_LEQUAL); GCE;
            RenderCommand::DrawIndexed(Renderer::GetRendererData().BoxMesh->GetVertexArray(), 0);
            glDepthFunc(GL_LESS); GCE;
        });
    }
}
