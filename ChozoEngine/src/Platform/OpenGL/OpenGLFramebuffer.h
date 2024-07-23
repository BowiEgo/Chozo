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
        virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

		virtual RendererID GetRendererID() const override { return m_RendererID; };
        virtual RendererID GetColorAttachmentRendererID(uint32_t attachmentIndex = 0) const override {
            CZ_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "attachmentIndex is smaller than colorAttachments size");
            return m_ColorAttachments[attachmentIndex];
        };
		virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; };

        virtual void ClearColorAttachmentBuffer(uint32_t attachmentIndex, const void* value) override;
		void Invalidate();
		void Release();
    private:
        FramebufferSpecification m_Specification;
        RendererID m_RendererID = 0;

        std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecs;
        FramebufferTextureSpecification m_DepthAttachmentSpec = FramebufferTextureFormat::None;

        std::vector<uint32_t> m_ColorAttachments;
        uint32_t m_DepthAttachment;
    };
}