#pragma once

#include "czpch.h"

#include "Shader/Shader.h"
#include "Framebuffer.h"
#include "Mesh.h"

namespace Chozo {

    struct PipelineSpecification
    {
        Ref<Shader> Shader;
        Ref<Framebuffer> TargetFramebuffer;
		VertexBufferLayout Layout;
        bool BackfaceCulling = true;
		bool DepthTest = true;
		bool DepthWrite = true;
		bool Wireframe = false;
		float LineWidth = 1.0f;

		std::string DebugName;
    };

    class Pipeline : public RefCounted
    {
    public:
        ~Pipeline() = default;

        virtual PipelineSpecification GetSpec() const = 0;
        virtual Ref<Shader> GetShader() const = 0;
        virtual Ref<Framebuffer> GetTargetFramebuffer() const = 0;

        static Ref<Pipeline> Create(PipelineSpecification& spec);
    };
}
