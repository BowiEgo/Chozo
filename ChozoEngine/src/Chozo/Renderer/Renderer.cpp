#include "Renderer.h"

#include "RendererAPI.h"
#include "Geometry/BoxGeometry.h"
#include "Geometry/QuadGeometry.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Chozo {

    static Renderer::RendererConfig s_Config;
    static Renderer::RendererData s_Data;
    static std::vector<std::function<void()>> s_RenderCommandQueue;

    RendererAPI* Renderer::s_RendererAPI;

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

        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!");
            case RendererAPI::API::OpenGL:   Renderer::s_RendererAPI = new OpenGLRendererAPI;
        }

        // Textures
        s_Data.MaxTextureSlots = RenderCommand::GetMaxTextureSlots();
        s_Data.TextureSlots.resize(s_Data.MaxTextureSlots);
        s_Data.WhiteTexture = Texture2D::Create();
        uint32_t whiteTextureData = 0xffffffff;
        s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

        TextureSpecification spec;
		spec.Format = ImageFormat::RGBA;
		spec.Width = 1;
		spec.Height = 1;
		s_Data.BlackTextureCube = TextureCube::Create(spec);
		constexpr uint32_t blackCubeTextureData[6] = { 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000 };
		s_Data.BlackTextureCube->SetData((void*)&blackCubeTextureData, sizeof(blackCubeTextureData));
        {
            const uint32_t cubemapSize = Renderer::GetConfig().EnvironmentMapResolution;
            TextureSpecification cubemapSpec;
            cubemapSpec.Format = ImageFormat::RGBA32F;
            cubemapSpec.Width = cubemapSize;
            cubemapSpec.Height = cubemapSize;
            s_Data.PreethamSkyTextureCube = TextureCube::Create(cubemapSpec);
            s_Data.StaticSkyTextureCube = TextureCube::Create(cubemapSpec);

            cubemapSpec.Width = 32;
            cubemapSpec.Height = 32;
            s_Data.IrradianceTextureCube = TextureCube::Create(cubemapSpec);

            cubemapSpec.Width = 128;
            cubemapSpec.Height = 128;
            s_Data.PrefilteredTextureCube = TextureCube::Create(cubemapSpec);
        }

        int32_t samplers[s_Data.MaxTextureSlots];
        for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
        {
            samplers[i] = i;
            s_Data.TextureSlots[i] = s_Data.WhiteTexture;
        }

        // Geometry
        s_Data.QuadMesh = std::make_shared<DynamicMesh>(static_cast<Ref<MeshSource>>(std::make_shared<QuadGeometry>()));
        s_Data.BoxMesh = std::make_shared<DynamicMesh>(static_cast<Ref<MeshSource>>(std::make_shared<BoxGeometry>()));

        // Shaders
        std::vector<int> samplersVec(samplers, samplers + s_Data.MaxTextureSlots);
        s_Data.m_ShaderLibrary = ShaderLibrary::Create();
        s_Data.m_ShaderLibrary->Load("Basic", "../assets/shaders/Basic.glsl.vert", "../assets/shaders/Basic.glsl.frag");
        s_Data.m_ShaderLibrary->Get("Basic")->Bind();
        s_Data.m_ShaderLibrary->Get("Basic")->SetUniform("u_Textures", samplersVec, s_Data.MaxTextureSlots);

        s_Data.m_ShaderLibrary->Load("Geometry", "../assets/shaders/GBuffer.glsl.vert", "../assets/shaders/GBuffer.glsl.frag");

        s_Data.m_ShaderLibrary->Load("Depth", "../assets/shaders/Basic.glsl.vert", "../assets/shaders/Depth.glsl.frag");
        
        s_Data.m_ShaderLibrary->Load("PhongLight", "../assets/shaders/FullScreenQuad.glsl.vert", "../assets/shaders/PhongLight.glsl.frag");
        s_Data.m_ShaderLibrary->Load("IrradianceConvolution", "../assets/shaders/CubemapSampler.glsl.vert", "../assets/shaders/IrradianceConvolution.glsl.frag");
        s_Data.m_ShaderLibrary->Load("Prefiltered", "../assets/shaders/CubemapSampler.glsl.vert", "../assets/shaders/Prefiltered.glsl.frag");
        s_Data.m_ShaderLibrary->Load("BRDF", "../assets/shaders/FullScreenQuad.glsl.vert", "../assets/shaders/BRDF.glsl.frag");
        s_Data.m_ShaderLibrary->Load("PBR", "../assets/shaders/FullScreenQuad.glsl.vert", "../assets/shaders/PBR.glsl.frag");

        s_Data.m_ShaderLibrary->Load("CubemapSampler", "../assets/shaders/CubemapSampler.glsl.vert", "../assets/shaders/CubemapSampler.glsl.frag");
        s_Data.m_ShaderLibrary->Load("PreethamSky", "../assets/shaders/PreethamSky.glsl.vert", "../assets/shaders/PreethamSky.glsl.frag");
        s_Data.m_ShaderLibrary->Load("Skybox", "../assets/shaders/Skybox.glsl.vert", "../assets/shaders/Skybox.glsl.frag");

        s_Data.m_ShaderLibrary->Load("CubemapPreview", "../assets/shaders/FullScreenQuad.glsl.vert", "../assets/shaders/CubemapPreview.glsl.frag");
        s_Data.m_ShaderLibrary->Load("SceneComposite", "../assets/shaders/FullScreenQuad.glsl.vert", "../assets/shaders/SceneComposite.glsl.frag");

        // PreethamSky
        {
            Ref<Shader> shader = Renderer::GetRendererData().m_ShaderLibrary->Get("PreethamSky");

            FramebufferSpecification fbSpec;
            fbSpec.Width = s_Data.PreethamSkyTextureCube->GetWidth();
            fbSpec.Height = s_Data.PreethamSkyTextureCube->GetHeight();
            fbSpec.Attachments = { ImageFormat::RGBA8 };
            Ref<Framebuffer> framebuffer = Framebuffer::Create(fbSpec);

            PipelineSpecification pipelineSpec;
			pipelineSpec.DebugName = "PreethamSky";
			pipelineSpec.Shader = shader;
            pipelineSpec.DepthWrite = false;
			pipelineSpec.DepthTest = false;
            pipelineSpec.Layout = {
				{ ShaderDataType::Float3, "a_Position" },
			};
			pipelineSpec.TargetFramebuffer = framebuffer;
            s_Data.m_PreethamSkyPipeline = Pipeline::Create(pipelineSpec);
        }
        // Cubemap-Sampler
        {
            Ref<Shader> shader = Renderer::GetRendererData().m_ShaderLibrary->Get("CubemapSampler");
            const uint32_t cubemapSize = Renderer::GetConfig().EnvironmentMapResolution;

            FramebufferSpecification fbSpec;
            fbSpec.Width = cubemapSize;
            fbSpec.Height = cubemapSize;
            fbSpec.Attachments = { ImageFormat::RGB16F };
            Ref<Framebuffer> framebuffer = Framebuffer::Create(fbSpec);

            PipelineSpecification pipelineSpec;
			pipelineSpec.DebugName = "CubemapSampler";
			pipelineSpec.Shader = shader;
            pipelineSpec.DepthWrite = false;
			pipelineSpec.DepthTest = false;
            pipelineSpec.Layout = {
				{ ShaderDataType::Float3, "a_Position" },
			};
			pipelineSpec.TargetFramebuffer = framebuffer;
            s_Data.m_CubemapSamplerPipeline = Pipeline::Create(pipelineSpec);
        }
        // Irradiance Map
        {
            FramebufferSpecification fbSpec;
            fbSpec.Width = 32;
            fbSpec.Height = 32;
			fbSpec.Attachments = { ImageFormat::RGB16F };
			Ref<Framebuffer> framebuffer = Framebuffer::Create(fbSpec);

            PipelineSpecification irradiancePipelineSpec;
			irradiancePipelineSpec.DebugName = "PBR-Irradiance";
			irradiancePipelineSpec.TargetFramebuffer = framebuffer;
            irradiancePipelineSpec.Shader = Renderer::GetShaderLibrary()->Get("IrradianceConvolution");
			s_Data.m_IrradianceMaterial = Material::Create(irradiancePipelineSpec.Shader, irradiancePipelineSpec.DebugName);
			s_Data.m_IrradiancePipeline = Pipeline::Create(irradiancePipelineSpec);
        }
        // Prefiltered Map
        {
            FramebufferSpecification fbSpec;
            fbSpec.Width = 128;
            fbSpec.Height = 128;
			fbSpec.Attachments = { ImageFormat::RGB16F };
			Ref<Framebuffer> framebuffer = Framebuffer::Create(fbSpec);

            PipelineSpecification prefilteredPipelineSpec;
			prefilteredPipelineSpec.DebugName = "PBR-Prefiltered";
			prefilteredPipelineSpec.TargetFramebuffer = framebuffer;
            prefilteredPipelineSpec.Shader = Renderer::GetShaderLibrary()->Get("Prefiltered");
			s_Data.m_PrefilteredMaterial = Material::Create(prefilteredPipelineSpec.Shader, prefilteredPipelineSpec.DebugName);
			s_Data.m_PrefilteredPipeline = Pipeline::Create(prefilteredPipelineSpec);
        }
        // BRDF-Texture
        {
            Ref<Shader> shader = Renderer::GetRendererData().m_ShaderLibrary->Get("BRDF");
            FramebufferSpecification fbSpec;
            fbSpec.Width = Renderer::GetConfig().IrradianceMapComputeSamples;
            fbSpec.Height = Renderer::GetConfig().IrradianceMapComputeSamples;
            fbSpec.Attachments = { ImageFormat::RG16F };
            Ref<Framebuffer> framebuffer = Framebuffer::Create(fbSpec);

            PipelineSpecification pipelineSpec;
			pipelineSpec.DebugName = "BRDF";
			pipelineSpec.Shader = shader;
            pipelineSpec.DepthWrite = false;
			pipelineSpec.DepthTest = false;
            pipelineSpec.Layout = {
				{ ShaderDataType::Float3, "a_Position" },
			};
			pipelineSpec.TargetFramebuffer = framebuffer;
            s_Data.m_BRDFLutPipeline = Pipeline::Create(pipelineSpec);

            framebuffer->Bind();
            s_RendererAPI->RenderFullscreenQuad(s_Data.m_BRDFLutPipeline);
            framebuffer->Unbind();
        }
        
    }

    void Renderer::RenderStaticBatches()
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
          
            Ref<Shader> shader = s_Data.m_ShaderLibrary->Get("Basic");
            shader->Bind();
            shader->SetUniform("u_VertUniforms.ModelMatrix", glm::mat4(1.0));
            shader->SetUniform("u_FragUniforms.Color", glm::vec4(0.1f, 0.5f, 1.0f, 1.0f));

            RenderCommand::DrawIndexed(pair.second->VAO, indexCount * 3);
            s_Data.Stats.DrawCalls++;
            s_Data.IndexCount += indexCount;
            s_Data.Stats.VerticesCount += vertexCount;
            s_Data.Stats.TriangleCount += indexCount;
        }
    }

    bool Renderer::SubmitStaticMesh(StaticMesh* mesh)
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
            mesh->OnSubmit(false);
            return false;
        }
        mesh->SetBufferSegmentID(segmentID);
        mesh->OnSubmit(true);
        return true;
    }

    bool Renderer::RemoveStaticMesh(StaticMesh* mesh)
    {
        return s_Data.BatchManager.RemoveSegment(mesh->GetBufferSegmentID());
    }

    void Renderer::DrawMesh(const glm::mat4 transform, DynamicMesh* mesh, Material* material, uint32_t entityID)
    {
        Ref<Shader> shader = material->GetShader();
        shader->Bind();
        for (auto& pair : material->GetUniforms())
            shader->SetUniform(pair.first, pair.second);
        shader->SetUniform("u_VertUniforms.ModelMatrix", transform);

        uint32_t indexCount = mesh->GetMeshSource()->GetIndexs().size();
        uint32_t vertexCount = mesh->GetMeshSource()->GetVertexs().size();

        RenderCommand::DrawIndexed(mesh->GetVertexArray(), indexCount * 3);
        s_Data.Stats.DrawCalls++;
        s_Data.IndexCount += indexCount;
        s_Data.Stats.VerticesCount += vertexCount;
        s_Data.Stats.TriangleCount += indexCount;
    }

    void Renderer::BeginRenderPass(Ref<RenderCommandBuffer> commandBuffer, Ref<RenderPass> renderPass)
    {
		s_RendererAPI->BeginRenderPass(commandBuffer, renderPass);
    }

    void Renderer::EndRenderPass(Ref<RenderCommandBuffer> commandBuffer, Ref<RenderPass> renderPass)
    {
		s_RendererAPI->EndRenderPass(commandBuffer, renderPass);
    }

    void Renderer::SubmitCubeMap(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<TextureCube> cubemap, Ref<Material> material)
    {
		s_RendererAPI->SubmitCubeMap(commandBuffer, pipeline, cubemap, material);
    }

    void Renderer::SubmitFullscreenQuad(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<Material> material)
    {
		s_RendererAPI->SubmitFullscreenQuad(commandBuffer, pipeline, material);
    }

    void Renderer::SubmitFullscreenBox(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<Material> material)
    {
		s_RendererAPI->SubmitFullscreenBox(commandBuffer, pipeline, material);
    }

    void Renderer::SubmitMeshWithMaterial(Ref<RenderCommandBuffer> commandBuffer, Ref<Pipeline> pipeline, Ref<DynamicMesh> mesh, Ref<Material> material)
    {
        s_RendererAPI->SubmitMeshWithMaterial(commandBuffer, pipeline, mesh, material);
    }

    void Renderer::Begin()
    {
        s_RenderCommandQueue.clear();
    }

    void Renderer::End()
    {
        for (auto& cmd : s_RenderCommandQueue)
            cmd();
    }

    void Renderer::Submit(std::function<void()> &&func)
    {
        s_RenderCommandQueue.push_back([func = std::forward<std::function<void()>>(func)]() mutable { func(); });
    }

    Renderer::RendererData Renderer::GetRendererData()
    {
        return s_Data;
    }

    Ref<Texture2D> Renderer::GetBRDFLutTexture()
    {
        return s_Data.m_BRDFLutPipeline->GetTargetFramebuffer()->GetImage(0);
    }

    Ref<TextureCube> Renderer::GetBlackTextureCube()
    {
        return s_Data.BlackTextureCube;
    }

    Ref<TextureCube> Renderer::GetStaticSkyTextureCube()
    {
        return s_Data.StaticSkyTextureCube;
    }

    Ref<TextureCube> Renderer::GetIrradianceTextureCube()
    {
        return s_Data.IrradianceTextureCube;
    }

    Ref<TextureCube> Renderer::GetPrefilteredTextureCube()
    {
        return s_Data.PrefilteredTextureCube;
    }

    Ref<TextureCube> Renderer::GetPreethamSkyTextureCube()
    {
        return s_Data.PreethamSkyTextureCube;
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

    Renderer::RendererConfig& Renderer::GetConfig()
    {
        return s_Config;
    }

    void Renderer::SetConfig(const Renderer::RendererConfig& config)
    {
        s_Config = config;
    }

    uint32_t Renderer::GetMaxTextureSlots()
    {
        return s_RendererAPI->GetMaxTextureSlots();
    }

    void Renderer::CreateStaticSky(const std::string &filePath)
    {
        s_RendererAPI->RenderCubemap(s_Data.m_CubemapSamplerPipeline, s_Data.StaticSkyTextureCube, filePath);

        s_Data.m_IrradianceMaterial->Set("u_Texture", s_Data.StaticSkyTextureCube);
        s_RendererAPI->RenderCubemap(s_Data.m_IrradiancePipeline, s_Data.IrradianceTextureCube, s_Data.m_IrradianceMaterial);

        s_Data.m_PrefilteredMaterial->Set("u_Texture", s_Data.PrefilteredTextureCube);
        s_RendererAPI->RenderCubemap(s_Data.m_PrefilteredPipeline, s_Data.PrefilteredTextureCube, s_Data.m_PrefilteredMaterial);
    }

    void Renderer::CreatePreethamSky(const float turbidity, const float azimuth, const float inclination)
    {
        s_RendererAPI->CreatePreethamSky(s_Data.m_PreethamSkyPipeline, turbidity, azimuth, inclination);
    }

    void Renderer::UpdatePreethamSky(const float turbidity, const float azimuth, const float inclination)
    {
        s_RendererAPI->DrawPreethamSky(s_Data.m_PreethamSkyPipeline, turbidity, azimuth, inclination);
    }
}