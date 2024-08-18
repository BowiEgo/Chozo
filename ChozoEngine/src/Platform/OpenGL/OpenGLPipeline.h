#pragma once

#include "Chozo/Renderer/Pipeline.h"

namespace Chozo
{

    class OpenGLPipeline : public Pipeline
    {
    public:
        OpenGLPipeline(PipelineSpecification& spec);
        ~OpenGLPipeline() = default;

        virtual inline Ref<Shader> GetShader() const override { return m_Shader; }
        virtual inline Ref<Framebuffer> GetTargetFramebuffer() const override { return m_TargetFramebuffer; }
        virtual inline Ref<DynamicMesh> GetDynamicMesh() const override { return m_DynamicMesh; }

		template<typename FuncT>
        void Submit(FuncT&& func) { m_RenderFunction = func; }
        void Submit() override {
            Submit([]() {
                std::cout << "Default Submit Behavior" << std::endl;
            });
        }

        void Begin();
        void End();
        void Render();
    private:
        Ref<Shader> m_Shader;
        Ref<Framebuffer> m_TargetFramebuffer;
        Ref<DynamicMesh> m_DynamicMesh;
        std::function<void()> m_RenderFunction;
    };
}
