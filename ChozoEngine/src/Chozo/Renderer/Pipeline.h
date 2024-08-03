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

        static Ref<Pipeline> Create();
    };
}
