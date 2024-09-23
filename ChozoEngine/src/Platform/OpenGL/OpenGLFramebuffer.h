#pragma once

#include "Chozo/Renderer/Framebuffer.h"
#include "Chozo/Renderer/Texture.h"

namespace Chozo {
    
    class OpenGLFramebuffer : public Framebuffer
    {
    public:
        OpenGLFramebuffer(const FramebufferSpecification& spec);
        virtual ~OpenGLFramebuffer();

        void Invalidate();
		void Release();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual void Resize(uint32_t width, uint32_t height, int mip = -1) override;
        virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

		virtual RendererID GetRendererID() const override { return m_RendererID; };
        virtual RendererID GetColorAttachmentRendererID(uint32_t attachmentIndex = 0) const override {
            CZ_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "attachmentIndex is smaller than colorAttachments size");
            return m_ColorAttachments[attachmentIndex];
        };
        virtual RendererID GetDepthAttachmentRendererID() const override { return m_DepthAttachment; }
		virtual FramebufferSpecification& GetSpecification() override { return m_Specification; };
        virtual Ref<Texture2D> GetImage(uint32_t attachmentIndex) const override { return m_ColorAttachmentImages[attachmentIndex]; }
		virtual Ref<Texture2D> GetDepthImage() const override { return m_DepthAttachmentImage; }
#if 0
        virtual void ClearColorAttachmentBuffer(uint32_t attachmentIndex) override;
#endif
    private:
        void CreateColorAttachmentImages(int samples, std::vector<FramebufferTextureSpecification> attachmentSpecs, uint32_t width, uint32_t height);
        void CreateDepthAttachmentImage(int samples, FramebufferTextureSpecification attachmentSpec, uint32_t width, uint32_t height);
    private:
        FramebufferSpecification m_Specification;
        RendererID m_RendererID = 0;

        std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecs;
        FramebufferTextureSpecification m_DepthAttachmentSpec = ImageFormat::None;

        std::vector<RendererID> m_ColorAttachments;
        RendererID m_DepthAttachment;

		std::vector<Ref<Texture2D>> m_ColorAttachmentImages;
		Ref<Texture2D> m_DepthAttachmentImage;
    };
}