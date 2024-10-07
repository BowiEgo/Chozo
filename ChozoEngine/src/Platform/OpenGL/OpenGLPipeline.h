#pragma once

#include "Chozo/Renderer/Pipeline.h"
#include "OpenGLUniformBuffer.h"

namespace Chozo
{

    class OpenGLPipeline : public Pipeline
    {
    public:
        OpenGLPipeline(PipelineSpecification& spec);
        ~OpenGLPipeline() = default;

        virtual PipelineSpecification GetSpec() const override { return m_Spec; }
        virtual inline Ref<Shader> GetShader() const override { return m_Shader; }
        virtual inline Ref<Framebuffer> GetTargetFramebuffer() const override { return m_TargetFramebuffer; }

        void BindUniformBlock();
    private:
        PipelineSpecification m_Spec;
        Ref<Shader> m_Shader;
        Ref<Framebuffer> m_TargetFramebuffer;
        std::unordered_map<std::string, Ref<OpenGLUniformBuffer>> m_UBs;

        friend class OpenGLRenderPass;
    };
}
