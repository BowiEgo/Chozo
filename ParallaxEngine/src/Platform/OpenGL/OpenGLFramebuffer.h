#pragma once

#include "Parallax/Renderer/Framebuffer.h"

namespace Parallax {
    
    class OpenGLFramebuffer : public Framebuffer
    {
    public:
        OpenGLFramebuffer(const FramebufferSpecification& spec);
        virtual ~OpenGLFramebuffer();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual void Resize(uint32_t width, uint32_t height) override;

        virtual uint32_t GetWidth() const override { return m_Width; };
		virtual uint32_t GetHeight() const override { return m_Height; };
		virtual RendererID GetRendererID() const override { return m_RendererID; };
        virtual RendererID GetColorAttachmentRendererID() const override { return m_ColorAttachment; };
		virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; };

		void Invalidate();
		void Release();
    private:
        FramebufferSpecification m_Specification;
        RendererID m_RendererID = 0;
        RendererID m_ColorAttachment = 0, m_DepthAttachment = 0;
		uint32_t m_Width = 0, m_Height = 0;

    };
}