#pragma once

#include "czpch.h"

#include "Shader.h"
#include "Framebuffer.h"
#include "UniformBuffer.h"
#include "Pipeline.h"
#include "Texture.h"

namespace Chozo {

    struct RenderPassSpecification
    {
        Ref<Pipeline> Pipeline;
		std::string DebugName;
    };

    class RenderPass
    {
    public:
        ~RenderPass() = default;
        virtual RenderPassSpecification GetSpecification() const = 0;

		virtual void SetInput(std::string_view name, Ref<UniformBuffer> uniformBuffer) = 0;
		virtual void SetInput(std::string_view name, Ref<TextureCube> textureCube) = 0;

        virtual std::unordered_map<std::string, Ref<UniformBuffer>> GetUniformBuffers() = 0;

		virtual Ref<Texture2D> GetOutput(uint32_t index) = 0;

        virtual Ref<Pipeline> GetPipeline() const = 0;
        virtual Ref<Framebuffer> GetTargetFramebuffer() const = 0;

        virtual void Bake() = 0;

        static Ref<RenderPass> Create(RenderPassSpecification& spec);
    };
}
