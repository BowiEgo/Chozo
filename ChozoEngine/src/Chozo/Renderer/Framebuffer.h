#pragma once

#include "czpch.h"
#include <glm/glm.hpp>

#include "Chozo/Renderer/RendererTypes.h"

namespace Chozo {
    struct FramebufferSpecification
    {
        uint32_t Width = 1;
        uint32_t Height = 1;
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

        virtual void Resize(float& width, float& height) = 0;

        virtual void ClearIDBuffer() = 0;

		virtual RendererID GetRendererID() const = 0;
        virtual RendererID GetColorAttachmentRendererID() const = 0;
		virtual const FramebufferSpecification& GetSpecification() const = 0;

        static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
    };
}