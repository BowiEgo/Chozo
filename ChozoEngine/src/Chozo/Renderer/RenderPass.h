#pragma once

#include "czpch.h"

#include "Shader.h"
#include "Framebuffer.h"
#include "Pipeline.h"
#include "Texture.h"

namespace Chozo {

    struct RenderPassSpecification
    {
        Ref<Pipeline> Pipeline;
    };

    class RenderPass
    {
    public:
        ~RenderPass() = default;
        virtual RenderPassSpecification GetSpecification() const = 0;

		virtual void SetInput(std::string_view name, Ref<TextureCube> textureCube) = 0;
		virtual void SetOutput(std::string_view name, Ref<TextureCube> textureCube) = 0;
		virtual Ref<TextureCube> GetOutput(std::string_view name) = 0;
        
        virtual Ref<Framebuffer> GetTargetFramebuffer() const = 0;

        virtual void Bake() = 0;

        static Ref<RenderPass> Create(RenderPassSpecification& spec);
    };
}
