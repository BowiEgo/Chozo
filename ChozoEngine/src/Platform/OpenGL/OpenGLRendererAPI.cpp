#include "OpenGLRendererAPI.h"

#include "OpenGLUtils.h"

#include "Chozo/Renderer/Renderer.h"
#include "Chozo/Renderer/Mesh.h"
// #include "OpenGLFrameBuffer.h"
#include "OpenGLRenderPass.h"

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

    void OpenGLRendererAPI::RenderPreethamSky(float turbidity, float azimuth, float inclination)
    {
        Ref<Shader> preethamSkyShader = Renderer::GetRendererData().m_PreethamSkyRenderPass->GetSpecification().Pipeline->GetShader();
        preethamSkyShader->Bind();
        preethamSkyShader->UploadUniformFloat("uniforms.Turbidity", turbidity);
        preethamSkyShader->UploadUniformFloat("uniforms.Azimuth", azimuth);
        preethamSkyShader->UploadUniformFloat("uniforms.Inclination", inclination);

        Ref<RenderPass> renderPass = Renderer::GetRendererData().m_PreethamSkyRenderPass;
        renderPass->Bake();
    }
}
