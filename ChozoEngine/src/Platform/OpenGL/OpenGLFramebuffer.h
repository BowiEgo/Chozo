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
        virtual RendererID GetColorAttachmentRendererID() const override { return m_ColorAttachment; };
		virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; };

        virtual void ClearIDBuffer() override;
		void Invalidate();
		void Release();
    private:
        FramebufferSpecification m_Specification;
        RendererID m_RendererID = 0;
        RendererID m_ColorAttachment = 0, m_IDAttachment = 0, m_DepthAttachment = 0;
    };
}