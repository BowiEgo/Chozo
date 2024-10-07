#pragma once

#include "Chozo/Renderer/RenderPass.h"
#include "Chozo/Renderer/UniformBuffer.h"
#include "Chozo/Renderer/Texture.h"

namespace Chozo {
    
    class OpenGLRenderPass : public RenderPass
    {
    public:
        OpenGLRenderPass(const RenderPassSpecification& spec);
		virtual ~OpenGLRenderPass() {};
        inline virtual RenderPassSpecification GetSpecification() const override { return m_Specification; }

		virtual void SetInput(std::string_view name, Ref<UniformBuffer> uniformBuffer) override;
        virtual void SetInput(std::string_view name, Ref<TextureCube> textureCube) override;

		virtual Ref<Texture2D> GetOutput(uint32_t index) override;
        
        virtual Ref<Pipeline> GetPipeline() const override;
        virtual Ref<Framebuffer> GetTargetFramebuffer() const override;

		virtual void Bake() override;
    private:
		RenderPassSpecification m_Specification;
    };
}