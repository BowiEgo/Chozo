#pragma once

#include "Chozo/Renderer/Pipeline.h"

namespace Chozo
{

    class OpenGLPipeline : public Pipeline
    {
    public:
        OpenGLPipeline(PipelineSpecification& spec);
        ~OpenGLPipeline() = default;

        virtual inline Ref<Framebuffer> GetTargetFramebuffer() const override { return m_TargetFramebuffer; }

        void Begin();
		template<typename FuncT>
        void Submit(FuncT&& func) {
            Begin();
            func();
            End();
        }
        void End();
    private:
        Ref<Shader> m_Shader;
        Ref<Framebuffer> m_TargetFramebuffer;
    };
}
