#pragma once

#include "Chozo/Renderer/Framebuffer.h"

namespace Chozo {
    
    class OpenGLFramebuffer : public Framebuffer
    {
    public:
        OpenGLFramebuffer(const FramebufferSpecification& spec);
        virtual ~OpenGLFramebuffer();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual void Resize(float& width, float& height) override;

		virtual RendererID GetRendererID() const override { return m_RendererID; };
        virtual RendererID GetColorAttachmentRendererID(uint32_t index = 0) const override {
            CZ_CORE_ASSERT(index < m_ColorAttachments.size(), "");
            return m_ColorAttachments[index];
        };
		virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; };

        virtual void ClearColorBuffer(int index, int value) override;
		void Invalidate();
		void Release();
    private:
        FramebufferSpecification m_Specification;
        RendererID m_RendererID = 0;

        std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecs;
        FramebufferTextureSpecification m_DepthAttachmentSpec = FramebufferTextureFormat::None;

        std::vector<RendererID> m_ColorAttachments;
        RendererID m_DepthAttachment;
    };
}