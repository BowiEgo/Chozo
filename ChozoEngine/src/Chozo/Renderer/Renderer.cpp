#include "Renderer.h"

#include "RenderCommand.h"
#include "Geometry/BoxGeometry.h"
#include "Geometry/QuadGeometry.h"

namespace Chozo {

    static Renderer::RendererConfig s_Config;
    static Renderer::RendererData* s_Data = nullptr;
    static std::vector<std::function<void()>> s_RenderCommandQueue;
    static std::atomic<std::chrono::steady_clock::time_point> s_LastSubmitTime = std::chrono::steady_clock::now();
    static std::future<void> s_DebounceTask;

    void Renderer::Init()
    {
        RenderCommand::Init();
        s_Data = new RendererData();

        // Textures
        s_Data->MaxTextureSlots = RenderCommand::GetMaxTextureSlots();
        s_Data->TextureSlots.resize(s_Data->MaxTextureSlots);
        s_Data->WhiteTexture = Texture2D::Create();
        uint32_t whiteTextureData = 0xffffffff;
        s_Data->WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
        s_Data->CheckerboardTexture = Texture2D::Create(std::string("../resources/textures/CheckerboardTexture.png"));

        TextureCubeSpecification spec;
		spec.Format = ImageFormat::RGBA;
		spec.Width = 1;
		spec.Height = 1;
		s_Data->BlackTextureCube = TextureCube::Create(spec);
		constexpr uint32_t blackCubeTextureData[6] = { 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000 };
		s_Data->BlackTextureCube->SetData((void*)&blackCubeTextureData, sizeof(blackCubeTextureData));
        {
            const uint32_t cubemapSize = Renderer::GetConfig().EnvironmentMapResolution;
            TextureCubeSpecification cubemapSpec;
            cubemapSpec.Format = ImageFormat::RGB16F;
            cubemapSpec.Width = cubemapSize;
            cubemapSpec.Height = cubemapSize;
            s_Data->PreethamSkyTextureCube = TextureCube::Create(cubemapSpec);
            s_Data->StaticSkyTextureCube = TextureCube::Create(cubemapSpec);

            cubemapSpec.Width = 32;
            cubemapSpec.Height = 32;
            s_Data->IrradianceTextureCube = TextureCube::Create(cubemapSpec);

            cubemapSpec.Width = 128;
            cubemapSpec.Height = 128;
            cubemapSpec.Mipmap = true;
            cubemapSpec.MinFilter = ImageParameter::LINEAR_MIPMAP_LINEAR;
            s_Data->PrefilteredTextureCube = TextureCube::Create(cubemapSpec);
        }
        s_Data->BrdfLUT = Texture2D::Create(std::string("../resources/textures/brdfLUT.png"));

        uint32_t samplers[s_Data->MaxTextureSlots];
        for (uint32_t i = 0; i < s_Data->MaxTextureSlots; i++)
        {
            samplers[i] = i;
            s_Data->TextureSlots[i] = s_Data->WhiteTexture;
        }

        // Geometry
        s_Data->QuadMesh = Geometry::Create<QuadGeometry>();
        s_Data->BoxMesh = Geometry::Create<BoxGeometry>();

        // Shaders
        std::vector<int> samplersVec(samplers, samplers + s_Data->MaxTextureSlots);
        s_Data->m_ShaderLibrary = ShaderLibrary::Create();

        auto shaderDir = std::string(Utils::File::GetShaderSoureceDirectory());
        s_Data->m_ShaderLibrary->Load("Solid", { shaderDir + "/Common/Model.glsl.vert",  shaderDir + "/Solid.glsl.frag" });
        s_Data->m_ShaderLibrary->Load("ID", { shaderDir + "/Common/FullScreenQuad.glsl.vert",  shaderDir + "/ID.glsl.frag" });
        s_Data->m_ShaderLibrary->Load("Geometry", { shaderDir + "/GBuffer.glsl.vert",  shaderDir + "/GBuffer.glsl.frag" });
        s_Data->m_ShaderLibrary->Load("Depth", { shaderDir + "/Common/Model.glsl.vert",  shaderDir + "/Depth.glsl.frag" });
        s_Data->m_ShaderLibrary->Load("Phong", { shaderDir + "/Common/FullScreenQuad.glsl.vert",  shaderDir + "/Phong.glsl.frag" });
        s_Data->m_ShaderLibrary->Load("IrradianceConvolution", { shaderDir + "/Common/CubemapSampler.glsl.vert",  shaderDir + "/IrradianceConvolution.glsl.frag" });
        s_Data->m_ShaderLibrary->Load("Prefiltered", { shaderDir + "/Common/CubemapSampler.glsl.vert",  shaderDir + "/Prefiltered.glsl.frag" });
        s_Data->m_ShaderLibrary->Load("BrdfLUT", { shaderDir + "/Common/FullScreenQuad.glsl.vert",  shaderDir + "/BrdfLUT.glsl.frag" });
        s_Data->m_ShaderLibrary->Load("PBR", { shaderDir + "/Common/FullScreenQuad.glsl.vert",  shaderDir + "/PBR.glsl.frag" });
        s_Data->m_ShaderLibrary->Load("CubemapSampler", { shaderDir + "/Common/CubemapSampler.glsl.vert",  shaderDir + "/CubemapSampler.glsl.frag" });
        s_Data->m_ShaderLibrary->Load("PreethamSky", { shaderDir + "/Common/CubemapSampler.glsl.vert",  shaderDir + "/PreethamSky.glsl.frag" });
        s_Data->m_ShaderLibrary->Load("Skybox", { shaderDir + "/Skybox.glsl.vert",  shaderDir + "/Skybox.glsl.frag" });
        s_Data->m_ShaderLibrary->Load("CubemapPreview", { shaderDir + "/Common/FullScreenQuad.glsl.vert",  shaderDir + "/CubemapPreview.glsl.frag" });
        s_Data->m_ShaderLibrary->Load("SceneComposite", { shaderDir + "/Common/FullScreenQuad.glsl.vert",  shaderDir + "/SceneComposite.glsl.frag" });

        // PreethamSky
        {
            Ref<Shader> shader = Renderer::GetRendererData().m_ShaderLibrary->Get("PreethamSky");

            FramebufferSpecification fbSpec;
            fbSpec.Width = s_Data->PreethamSkyTextureCube->GetWidth();
            fbSpec.Height = s_Data->PreethamSkyTextureCube->GetHeight();
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
            s_Data->m_PreethamSkyPipeline = Pipeline::Create(pipelineSpec);
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
            s_Data->m_CubemapSamplerPipeline = Pipeline::Create(pipelineSpec);
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
			s_Data->m_IrradianceMaterial = Material::Create(irradiancePipelineSpec.Shader, irradiancePipelineSpec.DebugName);
            // s_Data->m_IrradianceMaterial->Set("u_FragUniforms.Samples", GetConfig().IrradianceMapComputeSamples);
			s_Data->m_IrradiancePipeline = Pipeline::Create(irradiancePipelineSpec);
        }
        // Prefiltered Map
        {
            FramebufferSpecification fbSpec;
            fbSpec.Width = 128;
            fbSpec.Height = 128;
			fbSpec.Attachments = { ImageFormat::RGB16F, ImageFormat::Depth };
            fbSpec.DepthRenderbuffer = true;
			Ref<Framebuffer> framebuffer = Framebuffer::Create(fbSpec);

            PipelineSpecification prefilteredPipelineSpec;
			prefilteredPipelineSpec.DebugName = "PBR-Prefiltered";
			prefilteredPipelineSpec.TargetFramebuffer = framebuffer;
            prefilteredPipelineSpec.Shader = Renderer::GetShaderLibrary()->Get("Prefiltered");
			s_Data->m_PrefilteredMaterial = Material::Create(prefilteredPipelineSpec.Shader, prefilteredPipelineSpec.DebugName);
			s_Data->m_PrefilteredPipeline = Pipeline::Create(prefilteredPipelineSpec);
        }
        // BrdfLUT-Texture
        {
            Ref<Shader> shader = Renderer::GetRendererData().m_ShaderLibrary->Get("BrdfLUT");
            FramebufferSpecification fbSpec;
            fbSpec.Width = Renderer::GetConfig().IrradianceMapComputeSamples;
            fbSpec.Height = Renderer::GetConfig().IrradianceMapComputeSamples;
            fbSpec.Attachments = { ImageFormat::RG16F };
            Ref<Framebuffer> framebuffer = Framebuffer::Create(fbSpec);

            PipelineSpecification pipelineSpec;
			pipelineSpec.DebugName = "BrdfLUT";
			pipelineSpec.Shader = shader;
            pipelineSpec.DepthWrite = false;
			pipelineSpec.DepthTest = false;
            pipelineSpec.Layout = {
				{ ShaderDataType::Float3, "a_Position" },
			};
			pipelineSpec.TargetFramebuffer = framebuffer;
            s_Data->m_BrdfLUTPipeline = Pipeline::Create(pipelineSpec);

            framebuffer->Bind();
            RenderCommand::RenderFullscreenQuad(s_Data->m_BrdfLUTPipeline);
            framebuffer->Unbind();
        }
        
    }

    void Renderer::Shutdown()
    {
        delete s_Data;
    }

    void Renderer::DrawMesh(const glm::mat4 &transform, const DynamicMesh* mesh, Material* material, uint32_t entityID)
    {
        Ref<Shader> shader = material->GetShader();
        shader->Bind();
        for (auto& pair : material->GetUniforms())
            shader->SetUniform(pair.first, pair.second);
        shader->SetUniform("u_VertUniforms.ModelMatrix", transform);

        uint32_t indexCount = mesh->GetMeshSource()->GetIndexs().size();
        uint32_t vertexCount = mesh->GetMeshSource()->GetVertexs().size();

        RenderCommand::DrawIndexed(mesh->GetVertexArray(), indexCount * 3);
        s_Data->Stats.DrawCalls++;
        s_Data->IndexCount += indexCount;
        s_Data->Stats.VerticesCount += vertexCount;
        s_Data->Stats.TriangleCount += indexCount;
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
        s_RenderCommandQueue.emplace_back([func = std::forward<std::function<void()>>(func)]() mutable { func(); });
    }

    void Renderer::DebouncedSubmit(std::function<void()> &&func, uint32_t delay)
    {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - s_LastSubmitTime.load()).count();

        if (s_DebounceTask.valid())
            s_DebounceTask.wait();

        if (elapsed < delay)
        {
            s_DebounceTask = std::async(std::launch::async, [func = std::move(func), delay]() mutable {
                std::this_thread::sleep_for(std::chrono::milliseconds(delay));
                Submit(std::move(func));
            });
        }
        else
            Submit(std::move(func));


        s_LastSubmitTime.store(std::chrono::steady_clock::now());
    }

    Renderer::RendererData Renderer::GetRendererData()
    {
        return *s_Data; 
    }

    Ref<Texture2D> Renderer::GetBrdfLUT()
    {
        // return s_Data->BrdfLUT;
        return s_Data->m_BrdfLUTPipeline->GetTargetFramebuffer()->GetImage(0);
    }

    Ref<Texture2D> Renderer::GetCheckerboardTexture()
    {
        return s_Data->CheckerboardTexture;
    }

    Ref<TextureCube> Renderer::GetBlackTextureCube()
    {
        return s_Data->BlackTextureCube;
    }

    Ref<TextureCube> Renderer::GetStaticSkyTextureCube()
    {
        return s_Data->StaticSkyTextureCube;
    }

    Ref<TextureCube> Renderer::GetIrradianceTextureCube()
    {
        return s_Data->IrradianceTextureCube;
    }

    Ref<TextureCube> Renderer::GetPrefilteredTextureCube()
    {
        return s_Data->PrefilteredTextureCube;
    }

    Ref<TextureCube> Renderer::GetPreethamSkyTextureCube()
    {
        return s_Data->PreethamSkyTextureCube;
    }

    Renderer::Statistics Renderer::GetStats()
    {
        return s_Data->Stats;
    }

    void Renderer::ResetStats()
    {
        s_Data->Stats.VerticesCount = 0;
        s_Data->Stats.TriangleCount = 0;
        s_Data->Stats.DrawCalls = 0;
    }

    void Renderer::UpdateMaxTriangles(uint32_t count)
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
        return RenderCommand::GetMaxTextureSlots();
    }

    void Renderer::CreateStaticSky(const Ref<Texture2D>& texture)
    {
        Submit([texture](){
            RenderCommand::RenderCubemap(s_Data->m_CubemapSamplerPipeline, s_Data->StaticSkyTextureCube, texture);

            s_Data->m_IrradianceMaterial->Set("u_Texture", s_Data->StaticSkyTextureCube);
            RenderCommand::RenderCubemap(s_Data->m_IrradiancePipeline, s_Data->IrradianceTextureCube, s_Data->m_IrradianceMaterial);

            s_Data->m_PrefilteredMaterial->Set("u_Texture", s_Data->StaticSkyTextureCube);
            RenderCommand::RenderPrefilteredCubemap(s_Data->m_PrefilteredPipeline, s_Data->PrefilteredTextureCube, s_Data->m_PrefilteredMaterial);
        });
    }

    void Renderer::CreatePreethamSky(const float turbidity, const float azimuth, const float inclination)
    {
        RenderCommand::CreatePreethamSky(s_Data->m_PreethamSkyPipeline, turbidity, azimuth, inclination);

        Submit([](){
            s_Data->m_IrradianceMaterial->Set("u_Texture", s_Data->PreethamSkyTextureCube);
            RenderCommand::RenderCubemap(s_Data->m_IrradiancePipeline, s_Data->IrradianceTextureCube, s_Data->m_IrradianceMaterial);

            s_Data->m_PrefilteredMaterial->Set("u_Texture", s_Data->PreethamSkyTextureCube);
            RenderCommand::RenderPrefilteredCubemap(s_Data->m_PrefilteredPipeline, s_Data->PrefilteredTextureCube, s_Data->m_PrefilteredMaterial);
        });
    }

    void Renderer::UpdatePreethamSky(const float turbidity, const float azimuth, const float inclination)
    {
        RenderCommand::DrawPreethamSky(s_Data->m_PreethamSkyPipeline, turbidity, azimuth, inclination);

        Submit([](){
            s_Data->m_IrradianceMaterial->Set("u_Texture", s_Data->PreethamSkyTextureCube);
            RenderCommand::RenderCubemap(s_Data->m_IrradiancePipeline, s_Data->IrradianceTextureCube, s_Data->m_IrradianceMaterial);

            s_Data->m_PrefilteredMaterial->Set("u_Texture", s_Data->PreethamSkyTextureCube);
            RenderCommand::RenderPrefilteredCubemap(s_Data->m_PrefilteredPipeline, s_Data->PrefilteredTextureCube, s_Data->m_PrefilteredMaterial);
        });
    }
}