#pragma once

#include "Chozo/Renderer/Pipeline.h"

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

		// template<typename FuncT>
        void Submit(std::function<void()>&& func) override { m_RenderFunction = func; }
        // void Submit() override {
        //     Submit([]() {
        //         std::cout << "Default Submit Behavior" << std::endl;
        //     });
        // }

        virtual void Begin() override;
        virtual void End() override;
        virtual void Render() override;
    private:
        PipelineSpecification m_Spec;
        Ref<Shader> m_Shader;
        Ref<Framebuffer> m_TargetFramebuffer;
        std::function<void()> m_RenderFunction;
    };
}
