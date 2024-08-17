#include "OpenGLRendererAPI.h"

#include "OpenGLUtils.h"

#include "Chozo/Renderer/Renderer.h"
#include "Chozo/Renderer/Mesh.h"
// #include "OpenGLFrameBuffer.h"
#include "OpenGLPipeline.h"

#include <glad/glad.h>

namespace Chozo {
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

    Ref<TextureCube> OpenGLRendererAPI::CreatePreethamSky(float turbidity, float azimuth, float inclination)
    {
		const uint32_t cubemapSize = Renderer::GetConfig().EnvironmentMapResolution;
		const uint32_t irradianceMapSize = 32;

		TextureSpecification cubemapSpec;
		cubemapSpec.Format = ImageFormat::RGBA32F;
		cubemapSpec.Width = cubemapSize;
		cubemapSpec.Height = cubemapSize;

		Ref<TextureCube> environmentMap = TextureCube::Create(cubemapSpec);

        ShaderSpecification shaderSpec;
        shaderSpec.VertexFilepath = "../assets/shaders/PreethamSky.glsl.vert";
        shaderSpec.FragmentFilepath = "../assets/shaders/PreethamSky.glsl.frag";
        Ref<Shader> preethamSkyShader = Shader::Create(shaderSpec);

        preethamSkyShader->Bind();
        preethamSkyShader->UploadUniformFloat("uniforms.Turbidity", turbidity);
        preethamSkyShader->UploadUniformFloat("uniforms.Azimuth", azimuth);
        preethamSkyShader->UploadUniformFloat("uniforms.Inclination", inclination);

        Ref<DynamicMesh> Box = std::make_shared<DynamicMesh>(static_cast<Ref<MeshSource>>(Geometry::Create(GeometryType::Box)));

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

        FramebufferSpecification fbSpec;
        fbSpec.Width = cubemapSize;
        fbSpec.Height = cubemapSize;
        fbSpec.Attachments = { ImageFormat::RGBA8 };
        Ref<Framebuffer> framebuffer = Framebuffer::Create(fbSpec);

        // Create and sumbit to a new pipeline to bake environmentMap.
        PipelineSpecification pipelineSpec;
        pipelineSpec.Shader = preethamSkyShader;
        pipelineSpec.TargetFramebuffer = framebuffer;
        Ref<OpenGLPipeline> pipeline = std::make_shared<OpenGLPipeline>(pipelineSpec);

        environmentMap->Bind();
        pipeline->Submit([preethamSkyShader, environmentMap, Box]()
        {
            for (uint32_t i = 0; i < 6; ++i)
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, environmentMap->GetRendererID(), 0); GCE;
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GCE;

                preethamSkyShader->Bind();
                preethamSkyShader->UploadUniformMat4("camera.ViewMatrix", captureViews[i]);
                preethamSkyShader->UploadUniformMat4("camera.ProjectionMatrix", captureProjection);

                RenderCommand::DrawIndexed(Box->GetVertexArray(), 0);
            }
        });

        return environmentMap;
    }
}
