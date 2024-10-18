#include "OpenGLRendererAPI.h"

#include "OpenGLUtils.h"

#include "Chozo/Renderer/Renderer.h"
#include "Chozo/Renderer/Renderer2D.h"
#include "Chozo/Renderer/Mesh.h"
#include "OpenGLPipeline.h"
#include "OpenGLRenderPass.h"
#include "OpenGLMaterial.h"
#include "OpenGLTexture.h"

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
        glEnable(GL_STENCIL_TEST); GCE;
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
        glClearColor(color.r, color.g, color.b, color.a); GCE;
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
        textureCube.As<OpenGLTextureCube>()->Bind();
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
        environment->IrradianceMap.As<OpenGLTextureCube>()->Bind();
        Ref<Shader> shader = Renderer::GetRendererData().m_ShaderLibrary->Get("Skybox");
        shader->Bind();

        shader->SetUniform("u_Texture", 0);
        shader->SetUniform("u_FragUniforms.Intensity", environmentIntensity);
        shader->SetUniform("u_FragUniforms.TextureLod", skyboxLod);

        // TODO: Change to pipeline context status.
        glDepthFunc(GL_LEQUAL); GCE;
        DrawIndexed(Renderer::GetRendererData().BoxMesh->GetVertexArray(), 0);
        ResetGLContext();
    }

    void OpenGLRendererAPI::BeginRenderPass(Ref<RenderCommandBuffer> commandBuffer, Ref<RenderPass> renderPass)
    {
        commandBuffer->AddCommand([renderPass, this]()
        {
            Ref<Framebuffer> fbo = renderPass->GetTargetFramebuffer();
            fbo->Bind();
            SetClearColor(fbo->GetSpecification().ClearColor);
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

    void OpenGLRendererAPI::RenderCubemap(Ref<Pipeline> pipeline, Ref<TextureCube> cubemap, const Ref<Texture2D> texture)
    {
        auto fbo = pipeline->GetTargetFramebuffer();
        auto shader = pipeline->GetShader();

        fbo->Bind();
        pipeline.As<OpenGLPipeline>()->BindUniformBlock();
        texture.As<OpenGLTexture2D>()->Bind();
        shader->Bind();
        shader->SetUniform("u_EquirectangularMap", 0);
        for (uint32_t i = 0; i < 6; i++)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap->GetRendererID(), 0); GCE;
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GCE;

            shader->SetUniform("u_Camera.ViewMatrix", CubeTextureCaptureViews[i]);
            shader->SetUniform("u_Camera.ProjectionMatrix", CubeTextureCaptureProjection);

            DrawIndexed(Renderer::GetRendererData().BoxMesh->GetVertexArray(), 0);
        }
        fbo->Unbind();
    }

    void OpenGLRendererAPI::RenderCubemap(Ref<Pipeline> pipeline, Ref<TextureCube> cubemap, Ref<Material> material)
    {
        auto fbo = pipeline->GetTargetFramebuffer();
        auto shader = pipeline->GetShader();

        fbo->Bind();
        pipeline.As<OpenGLPipeline>()->BindUniformBlock();
        if (material) { material.As<OpenGLMaterial>()->Bind(); }
        shader->Bind();
        for (uint32_t i = 0; i < 6; i++)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap->GetRendererID(), 0); GCE;
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GCE;

            shader->SetUniform("u_Camera.ViewMatrix", CubeTextureCaptureViews[i]);
            shader->SetUniform("u_Camera.ProjectionMatrix", CubeTextureCaptureProjection);

            DrawIndexed(Renderer::GetRendererData().BoxMesh->GetVertexArray(), 0);
        }
        fbo->Unbind();
    }

    void OpenGLRendererAPI::RenderPrefilteredCubemap(Ref<Pipeline> pipeline, Ref<TextureCube> cubemap, Ref<Material> material)
    {
        auto fbo = pipeline->GetTargetFramebuffer();
        auto shader = pipeline->GetShader();

        fbo->Bind();
        pipeline.As<OpenGLPipeline>()->BindUniformBlock();
        if (material) { material.As<OpenGLMaterial>()->Bind(); }
        shader->Bind();
        unsigned int maxMipLevels = 5;
        for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
        {
            // reisze framebuffer according to mip-level size.
            unsigned int mipWidth  = static_cast<unsigned int>(128 * std::pow(0.5, mip));
            unsigned int mipHeight = static_cast<unsigned int>(128 * std::pow(0.5, mip));
            fbo->Resize(mipWidth, mipHeight, mip);

            float roughness = (float)mip / (float)(maxMipLevels - 1);
            shader->SetUniform("u_FragUniforms.Roughness", roughness);
            for (unsigned int i = 0; i < 6; ++i)
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap->GetRendererID(), mip); GCE;

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GCE;
                shader->SetUniform("u_Camera.ViewMatrix", CubeTextureCaptureViews[i]);
                shader->SetUniform("u_Camera.ProjectionMatrix", CubeTextureCaptureProjection);
                DrawIndexed(Renderer::GetRendererData().BoxMesh->GetVertexArray(), 0);
            }
        }
        fbo->Unbind();
    }

    void OpenGLRendererAPI::DrawPreethamSky(Ref<Pipeline> pipeline, const float turbidity, const float azimuth, const float inclination)
    {
        auto fbo = pipeline->GetTargetFramebuffer();
        auto textureCube = Renderer::GetPreethamSkyTextureCube();
        auto shader = pipeline->GetShader();

        fbo->Bind();
        textureCube.As<OpenGLTextureCube>()->Bind();
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
        fbo->Unbind();
    }

    void OpenGLRendererAPI::SubmitCubeMap(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<TextureCube> cubemap, Ref<Material> material)
    {
        commandBuffer->AddCommand([pipeline, cubemap, material, this]()
        {
            auto shader = pipeline->GetShader();

            pipeline.As<OpenGLPipeline>()->BindUniformBlock();
            if (material) { material.As<OpenGLMaterial>()->Bind(); }
            shader->Bind();
            for (uint32_t i = 0; i < 6; i++)
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap->GetRendererID(), 0); GCE;
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GCE;

                shader->SetUniform("u_Camera.ViewMatrix", CubeTextureCaptureViews[i]);
                shader->SetUniform("u_Camera.ProjectionMatrix", CubeTextureCaptureProjection);

                DrawIndexed(Renderer::GetRendererData().BoxMesh->GetVertexArray(), 0);
            }
        });
    }

    void OpenGLRendererAPI::RenderFullscreenQuad(Ref<Pipeline> pipeline, Ref<Material> material)
    {
        auto shader = pipeline->GetShader();

        pipeline.As<OpenGLPipeline>()->BindUniformBlock();
        if (material) { material.As<OpenGLMaterial>()->Bind(); }
        shader->Bind();
        PrepareGLContext(pipeline);
        Renderer2D::DrawFullScreenQuad();
        ResetGLContext();
    }

    void OpenGLRendererAPI::SubmitFullscreenQuad(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<Material> material)
    {
        commandBuffer->AddCommand([pipeline, material, this]()
        {
            RenderFullscreenQuad(pipeline, material);
        });
    }

    void OpenGLRendererAPI::SubmitFullscreenBox(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<Material> material)
    {
        commandBuffer->AddCommand([pipeline, material, this]()
        {
            pipeline.As<OpenGLPipeline>()->BindUniformBlock();
            
            if (material) { material.As<OpenGLMaterial>()->Bind(); }

            PrepareGLContext(pipeline);
            RenderCommand::DrawIndexed(Renderer::GetRendererData().BoxMesh->GetVertexArray(), 0);
            ResetGLContext();
        });
    }

    void OpenGLRendererAPI::SubmitMeshWithMaterial(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<DynamicMesh> mesh, Ref<Material> material, glm::mat4 transform)
    {
        commandBuffer->AddCommand([pipeline, mesh, material, transform, this]()
        {
            auto shader = pipeline->GetShader();

            pipeline.As<OpenGLPipeline>()->BindUniformBlock();

            if (material)
            {
                material.As<OpenGLMaterial>()->Bind();
            }
            shader->Bind();
            shader->SetUniform("u_VertUniforms.ModelMatrix", transform);

            uint32_t indexCount = mesh->GetMeshSource()->GetIndexs().size();
            uint32_t vertexCount = mesh->GetMeshSource()->GetVertexs().size();

            glEnable(GL_CULL_FACE); GCE;
            RenderCommand::DrawIndexed(mesh->GetVertexArray(), indexCount * 3);
            glDisable(GL_CULL_FACE); GCE;

            auto rendererData = Renderer::GetRendererData();
            rendererData.Stats.DrawCalls++;
            rendererData.IndexCount += indexCount;
            rendererData.Stats.VerticesCount += vertexCount;
            rendererData.Stats.TriangleCount += indexCount;
        });
    }

    void OpenGLRendererAPI::CopyImage(Ref<RenderCommandBuffer> commandBuffer, Ref<Texture2D> source, SharedBuffer& dest)
    {
        commandBuffer->AddCommand([source, &dest]() mutable
        {
            source->ExtractBuffer();
            source->CopyToHostBuffer(dest);
        });
    }

    void OpenGLRendererAPI::PrepareGLContext(Ref<Pipeline> pipeline)
    {
        if (pipeline->GetSpec().DepthWrite)
            glDepthFunc(GL_LEQUAL); GCE;
    }

    void OpenGLRendererAPI::ResetGLContext()
    {
        glDepthFunc(GL_LESS); GCE;
    }
}
