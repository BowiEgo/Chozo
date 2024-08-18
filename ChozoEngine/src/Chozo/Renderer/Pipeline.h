#pragma once

#include "czpch.h"

#include "Shader.h"
#include "Framebuffer.h"
#include "Mesh.h"

namespace Chozo {

    struct PipelineSpecification
    {
        Ref<Shader> Shader;
        Ref<Framebuffer> TargetFramebuffer;
        Ref<DynamicMesh> DynamicMesh;
    };

    class Pipeline
    {
    public:
        ~Pipeline() = default;

        virtual Ref<Shader> GetShader() const = 0;
        virtual Ref<Framebuffer> GetTargetFramebuffer() const = 0;
        virtual Ref<DynamicMesh> GetDynamicMesh() const = 0;

        virtual void Submit() = 0;

        static Ref<Pipeline> Create(PipelineSpecification& spec);
    };
}
