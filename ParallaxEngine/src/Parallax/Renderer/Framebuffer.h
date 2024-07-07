#pragma once

#include "prxpch.h"
#include <glm/glm.hpp>

#include "Parallax/Renderer/RendererTypes.h"

namespace Parallax {
    struct FramebufferSpecification
    {
        uint32_t Width = 0;
        uint32_t Height = 0;
		glm::vec4 ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

		uint32_t Samples = 1; // multisampling
        bool HDR = false;

        // SwapChainTarget = screen buffer (i.e. no framebuffer)
		bool SwapChainTarget = false;
    };
    

    class Framebuffer
    {
    public:
        virtual ~Framebuffer() {}

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual void Resize(uint32_t width, uint32_t height) = 0;

        virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual RendererID GetRendererID() const = 0;
        virtual RendererID GetColorAttachmentRendererID() const = 0;
		virtual const FramebufferSpecification& GetSpecification() const = 0;

        static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
    };
}