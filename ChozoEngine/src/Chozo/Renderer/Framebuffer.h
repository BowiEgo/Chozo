#pragma once

#include "czpch.h"
#include <glm/glm.hpp>

#include "RendererTypes.h"
#include "Texture.h"

namespace Chozo {

    enum class FBObjectFormat
    {
        Texture, RenderBuffer
    };

    struct FramebufferTextureSpecification
    {
        FramebufferTextureSpecification() = default;
        FramebufferTextureSpecification(ImageFormat format)
            : TextureFormat(format) {};

        // FBObjectFormat ObjectFormat = FBObjectFormat::Texture;
        ImageFormat TextureFormat = ImageFormat::None;
        // TODO: filtering/wrap
    };

    struct FramebufferAttachmentSpecification
    {
        FramebufferAttachmentSpecification() = default;
        FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments)
            : Attachments(attachments) {};

        std::vector<FramebufferTextureSpecification> Attachments;
    };

    struct FramebufferSpecification
    {
        uint32_t Width = 1, Height = 1;
		glm::vec4 ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };

        FramebufferAttachmentSpecification Attachments;

		uint32_t Samples = 1; // multisampling
        bool HDR = false;
        bool DepthRenderbuffer = false;

        // SwapChainTarget = screen buffer (i.e. no framebuffer)
		bool SwapChainTarget = false;

		std::map<uint32_t, Ref<Texture2D>> ExistingImages;
    };
    

    class Framebuffer
    {
    public:
        virtual ~Framebuffer() {}

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual void Resize(uint32_t width, uint32_t height, int mip = -1) = 0;
        virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

        virtual void ClearColorAttachmentBuffer(uint32_t attachmentIndex) = 0;

		virtual RendererID GetRendererID() const = 0;
        virtual RendererID GetColorAttachmentRendererID(uint32_t attachmentIndex = 0) const = 0;
        virtual RendererID GetDepthAttachmentRendererID() const = 0;
		virtual FramebufferSpecification& GetSpecification() = 0;
        virtual Ref<Texture2D> GetImage(uint32_t attachmentIndex) const = 0;
		virtual Ref<Texture2D> GetDepthImage() const = 0;

        static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
    };
}