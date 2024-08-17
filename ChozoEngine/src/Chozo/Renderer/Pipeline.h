#pragma once

#include "czpch.h"

#include "Shader.h"
#include "Framebuffer.h"

namespace Chozo {

    struct PipelineSpecification
    {
        Ref<Shader> Shader;
        Ref<Framebuffer> TargetFramebuffer;
    };

    class Pipeline
    {
    public:
        ~Pipeline() = default;

        virtual Ref<Framebuffer> GetTargetFramebuffer() const = 0;

        static Ref<Pipeline> Create(PipelineSpecification& spec);
    };
}
