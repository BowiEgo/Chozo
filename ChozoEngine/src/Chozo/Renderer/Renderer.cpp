#include "Renderer.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Chozo {

    static Renderer::RendererConfig s_Config;
    static Renderer::RendererData s_Data;

    Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;

    template<typename T>
    static void SetMaxSize(T*& target, T*& ptr, uint32_t newSize)
    {
        T* newBuffer = new T[newSize];

        if (target)
        {
            std::copy(target, target + newSize, newBuffer);
            ptr = newBuffer + (ptr - target);
            delete[] target;
            target = nullptr;
        }
        else
        {
            ptr = newBuffer;
        }

        target = newBuffer;
    }

    void Renderer::Init()
    {
        RenderCommand::Init();

        // Textures
        s_Data.MaxTextureSlots = RenderCommand::GetMaxTextureSlots();
        s_Data.TextureSlots.resize(s_Data.MaxTextureSlots);
        s_Data.WhiteTexture = Texture2D::Create();
        uint32_t whiteTextureData = 0xffffffff;
        s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

        int32_t samplers[s_Data.MaxTextureSlots];
        for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
        {
            samplers[i] = i;
            s_Data.TextureSlots[i] = s_Data.WhiteTexture;
        }

        // Shader
        s_Data.m_ShaderLibrary = ShaderLibrary::Create();
        s_Data.m_ShaderLibrary->Load("Shader", "../assets/shaders/Shader.glsl.vert", "../assets/shaders/Shader.glsl.frag");
        s_Data.m_ShaderLibrary->Get("Shader")->Bind();
        s_Data.m_ShaderLibrary->Get("Shader")->UploadUniformIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);
        
        s_Data.m_ShaderLibrary->Load("Skybox", "../assets/shaders/Skybox.glsl.vert", "../assets/shaders/Skybox.glsl.frag");
        s_Data.m_ShaderLibrary->Load("PreethamSky", "../assets/shaders/PreethamSky.glsl.vert", "../assets/shaders/PreethamSky.glsl.frag");

        // Uniform buffer
        s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(RendererData::CameraData));

        // Skybox
        s_Data.SkyBoxMesh = std::make_shared<DynamicMesh>(static_cast<Ref<MeshSource>>(Geometry::Create(GeometryType::Box)));
        // PreethamSky
        {
            Ref<Shader> preethamSkyShader = Renderer::GetRendererData().m_ShaderLibrary->Get("PreethamSky");

            const uint32_t cubemapSize = Renderer::GetConfig().EnvironmentMapResolution;
            // const uint32_t irradianceMapSize = 32;

            TextureSpecification cubemapSpec;
            cubemapSpec.Format = ImageFormat::RGBA32F;
            cubemapSpec.Width = cubemapSize;
            cubemapSpec.Height = cubemapSize;
    		Ref<TextureCube> environmentMap = TextureCube::Create(cubemapSpec);

            FramebufferSpecification fbSpec;
            fbSpec.Width = cubemapSize;
            fbSpec.Height = cubemapSize;
            fbSpec.Attachments = { ImageFormat::RGBA8 };
            Ref<Framebuffer> framebuffer = Framebuffer::Create(fbSpec);

            PipelineSpecification pipelineSpec;
            pipelineSpec.Shader = preethamSkyShader;
            pipelineSpec.TargetFramebuffer = framebuffer;
            pipelineSpec.DynamicMesh = std::make_shared<DynamicMesh>(static_cast<Ref<MeshSource>>(Geometry::Create(GeometryType::Box)));
            Ref<Pipeline> preethamSkyPipeline = Pipeline::Create(pipelineSpec);

            RenderPassSpecification renderPassSpec;
            renderPassSpec.Pipeline = preethamSkyPipeline;
            s_Data.m_PreethamSkyRenderPass = RenderPass::Create(renderPassSpec);
            s_Data.m_PreethamSkyRenderPass->SetOutput("EnvMap", environmentMap);
        }
    }

    void Renderer::Shutdown()
    {
    }

    void Renderer::BeginScene(const glm::mat4& projection, const glm::mat4 &transform)
    {
        s_Data.CameraBuffer.ProjectionMatrix = projection;
        s_Data.CameraBuffer.ViewMatrix = glm::inverse(transform);
        s_Data.CameraBuffer.InverseViewProjectionMatrix = glm::inverse(s_Data.CameraBuffer.ProjectionMatrix * s_Data.CameraBuffer.ViewMatrix);
        s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(RendererData::CameraData));
    }

    void Renderer::BeginScene(EditorCamera& camera)
    {
        s_Data.CameraBuffer.ProjectionMatrix = camera.GetProjection();
        s_Data.CameraBuffer.ViewMatrix = camera.GetViewMatrix();
        s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(RendererData::CameraData));
    }

    void Renderer::EndScene()
    {
        RenderBatches();
    }

    void Renderer::RenderBatches()
    {
        s_Data.IndexCount = 0;
        for (const auto& pair : s_Data.BatchManager.GetRenderSources())
        {
            Ref<Batch<Vertex>> vertexBatch = s_Data.BatchManager.GetBatches<Vertex>()[pair.first];
            Ref<Batch<Index>> indexBatch = s_Data.BatchManager.GetBatches<Index>()[pair.first];

            uint32_t vertexCount = vertexBatch->GetCount();
            uint32_t indexCount = indexBatch->GetCount();

            if (vertexCount == 0 || indexCount == 0)
                continue;
          
            Ref<Shader> shader = s_Data.m_ShaderLibrary->Get("Shader");
            shader->Bind();
            shader->UploadUniformFloat4("uniforms.Color", glm::vec4(0.1f, 0.5f, 1.0f, 1.0f));
            RenderCommand::DrawIndexed(pair.second->VAO, indexCount * 3);
            s_Data.Stats.DrawCalls++;

            s_Data.IndexCount += indexCount;
            s_Data.Stats.VerticesCount += vertexCount;
            s_Data.Stats.TriangleCount += indexCount;
        }
    }

    bool Renderer::SubmitMesh(StaticMesh* mesh)
    {
        UUID segmentID = s_Data.BatchManager.SubmitBuffers(
            mesh->GetMeshSource()->GetTempBuffer()->Vertexs.data(),
            mesh->GetMeshSource()->GetTempBuffer()->Vertexs.size(),
            mesh->GetMeshSource()->GetTempBuffer()->Indexs.data(),
            mesh->GetMeshSource()->GetTempBuffer()->Indexs.size(),
            mesh->GetBufferSegmentID()
        );
        if (!segmentID.isValid())
        {
            return false;
        }
        mesh->SetBufferSegmentID(segmentID);
        return true;
    }

    bool Renderer::RemoveMesh(StaticMesh* mesh)
    {
        return s_Data.BatchManager.RemoveSegment(mesh->GetBufferSegmentID());
    }

    void Renderer::DrawMesh(const glm::mat4 transform, Ref<MeshSource> meshSource, uint32_t entityID)
    {
        // s_Data.Shader->Bind();
        // s_Data.Shader->UploadUniformMat4("u_ModelMatrix", transform);
        // RenderCommand::DrawIndexed(meshSource->GetVertexArray(), meshSource->GetIndexs().size() * 3);
        // s_Data.Stats.DrawCalls++;
    }

    Renderer::RendererData Renderer::GetRendererData()
    {
        return s_Data;
    }

    Renderer::Statistics Renderer::GetStats()
    {
        return s_Data.Stats;
    }

    void Renderer::ResetStats()
    {
        s_Data.Stats.VerticesCount = 0;
        s_Data.Stats.TriangleCount = 0;
        s_Data.Stats.DrawCalls = 0;
    }

    void Renderer::UpdateMaxTriagles(uint32_t count)
    {
    }

    void Renderer::RenderPreethamSky(float turbidity, float azimuth, float inclination)
    {
		RenderCommand::RenderPreethamSky(turbidity, azimuth, inclination);
    }

    Renderer::RendererConfig& Renderer::GetConfig()
    {
        return s_Config;
    }

    void Renderer::SetConfig(const Renderer::RendererConfig& config)
    {
        s_Config = config;
    }

    void Renderer::DrawSkyLight(Ref<Environment>& environment, float& environmentIntensity, float& skyboxLod, EditorCamera& camera)
    {
        glm::mat4 proj = camera.GetProjection();
        glm::mat4 view = glm::mat3(camera.GetViewMatrix());

        environment->IrradianceMap->Bind();
        Ref<Shader> shader = s_Data.m_ShaderLibrary->Get("Skybox");
        shader->Bind();

        shader->UploadUniformMat4("camera.ProjectionMatrix", proj);
        shader->UploadUniformMat4("camera.ViewMatrix", view);

        shader->UploadUniformInt("u_Texture", 0);
        shader->UploadUniformFloat("u_Uniforms.Intensity", environmentIntensity);
        shader->UploadUniformFloat("u_Uniforms.TextureLod", skyboxLod);

        // TODO: Remove OpenGL platform codes.
        glDepthFunc(GL_LEQUAL);
        RenderCommand::DrawIndexed(s_Data.SkyBoxMesh->GetVertexArray(), 0);
        glDepthFunc(GL_LESS);
    }
}