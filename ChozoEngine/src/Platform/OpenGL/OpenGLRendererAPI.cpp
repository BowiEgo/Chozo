#include "OpenGLRendererAPI.h"

#include "OpenGLUtils.h"

#include "Chozo/Renderer/Renderer.h"
#include "Chozo/Renderer/Renderer2D.h"
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

    uint32_t OpenGLRendererAPI::GetMaxTextureSlots()
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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); GCE;
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

            DrawIndexed(VAO, 0);
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
        DrawIndexed(Renderer::GetRendererData().BoxMesh->GetVertexArray(), 0);
        glDepthFunc(GL_LESS); GCE;
    }

    void OpenGLRendererAPI::BeginRenderPass(Ref<RenderCommandBuffer> commandBuffer, Ref<RenderPass> renderPass)
    {
        commandBuffer->AddCommand([renderPass, this]()
        {
            renderPass->GetTargetFramebuffer()->Bind();
            Clear();
        });
    }

    void OpenGLRendererAPI::EndRenderPass(Ref<RenderCommandBuffer> commandBuffer, Ref<RenderPass> renderPass)
    {
        commandBuffer->AddCommand([renderPass]()
        {
            renderPass->GetTargetFramebuffer()->Unbind();
        });
    }

    void OpenGLRendererAPI::CreatePreethamSky(Ref<Pipeline> pipeline, const float turbidity, const float azimuth, const float inclination)
    {
        Renderer::Submit([pipeline, turbidity, azimuth, inclination, this]()
        {
            pipeline->GetTargetFramebuffer()->Bind();
            DrawPreethamSky(pipeline, turbidity, azimuth, inclination);
            pipeline->GetTargetFramebuffer()->Unbind();
        });
    }

    Ref<TextureCube> OpenGLRendererAPI::CreateCubemap(Ref<Pipeline> pipeline, const std::string filePath)
    {
        CZ_CORE_INFO("CreateCubemap: {}", filePath);
        OpenGLShader* shader = static_cast<OpenGLShader*>(pipeline->GetShader().get());

        TextureSpecification spec;
        spec.WrapS = ImageParameter::CLAMP_TO_BORDER;
        spec.WrapT = ImageParameter::CLAMP_TO_BORDER;
        Ref<Texture2D> equirectangularMap = Texture2D::Create(filePath, spec);

        TextureSpecification cubemapSpec;
        cubemapSpec.Format = ImageFormat::RGBA32F;
        cubemapSpec.Width = pipeline->GetTargetFramebuffer()->GetSpecification().Width;
        cubemapSpec.Height = pipeline->GetTargetFramebuffer()->GetSpecification().Height;
        Ref<TextureCube> cubeMap = TextureCube::Create(cubemapSpec);

        pipeline->GetTargetFramebuffer()->Bind();
        equirectangularMap->Bind();
        shader->Bind();
        shader->SetUniform("u_EquirectangularMap", 0);

        for (uint32_t i = 0; i < 6; i++)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubeMap->GetRendererID(), 0); GCE;
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GCE;

            shader->SetUniform("u_Camera.ViewMatrix", CubeTextureCaptureViews[i]);
            shader->SetUniform("u_Camera.ProjectionMatrix", CubeTextureCaptureProjection);

            DrawIndexed(Renderer::GetRendererData().BoxMesh->GetVertexArray(), 0);
        }
        pipeline->GetTargetFramebuffer()->Unbind();

        return cubeMap;
    }

    void OpenGLRendererAPI::DrawPreethamSky(Ref<Pipeline> pipeline, const float turbidity, const float azimuth, const float inclination)
    {
        OpenGLShader* shader = static_cast<OpenGLShader*>(pipeline->GetShader().get());
        Ref<TextureCube> textureCube = Renderer::GetPreethamSkyTextureCube();

        pipeline->GetTargetFramebuffer()->Bind();
        textureCube->Bind();

        shader->Bind();
        shader->SetUniform("u_FragUniforms.Turbidity", turbidity);
        shader->SetUniform("u_FragUniforms.Azimuth", azimuth);
        shader->SetUniform("u_FragUniforms.Inclination", inclination);
        for (uint32_t i = 0; i < 6; i++)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, textureCube->GetRendererID(), 0); GCE;
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GCE;

            shader->SetUniform("u_Camera.ViewMatrix", CubeTextureCaptureViews[i]);
            shader->SetUniform("u_Camera.ProjectionMatrix", CubeTextureCaptureProjection);

            DrawIndexed(Renderer::GetRendererData().BoxMesh->GetVertexArray(), 0);
        }
        pipeline->GetTargetFramebuffer()->Unbind();
    }

    void OpenGLRendererAPI::SubmitFullscreenQuad(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<Material> material)
    {
        commandBuffer->AddCommand([pipeline, material]()
        {
            static_cast<OpenGLMaterial*>(material.get())->Bind();

            glDepthFunc(GL_LEQUAL); GCE;
            Renderer2D::DrawFullScreenQuad();
            glDepthFunc(GL_LESS); GCE;
        });
    }

    void OpenGLRendererAPI::SubmitFullscreenBox(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<Material> material)
    {
        commandBuffer->AddCommand([pipeline, material]()
        {
            static_cast<OpenGLMaterial*>(material.get())->Bind();

            glDepthFunc(GL_LEQUAL); GCE;
            RenderCommand::DrawIndexed(Renderer::GetRendererData().BoxMesh->GetVertexArray(), 0);
            glDepthFunc(GL_LESS); GCE;
        });
    }

    void OpenGLRendererAPI::SubmitMeshWithMaterial(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<DynamicMesh> mesh, Ref<Material> material)
    {
        commandBuffer->AddCommand([mesh, material, this]()
        {
            static_cast<OpenGLMaterial*>(material.get())->Bind();

            uint32_t indexCount = mesh->GetMeshSource()->GetIndexs().size();
            uint32_t vertexCount = mesh->GetMeshSource()->GetVertexs().size();

            RenderCommand::DrawIndexed(mesh->GetVertexArray(), indexCount * 3);

            auto rendererData = Renderer::GetRendererData();
            rendererData.Stats.DrawCalls++;
            rendererData.IndexCount += indexCount;
            rendererData.Stats.VerticesCount += vertexCount;
            rendererData.Stats.TriangleCount += indexCount;
        });
    }
}
