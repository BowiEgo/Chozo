#pragma once

#include "Uniform.h"
#include "Chozo/Renderer/Shader/ShaderReflection.h"
#include "Chozo/Renderer/RendererTypes.h"
#include "Chozo/Core/Thread.h"

#include "czpch.h"
#include <glm/glm.hpp>

namespace Chozo {


    class Shader : public RefCounted
    {
    public:
        virtual ~Shader() = default;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual const std::string& GetName() const = 0;
        virtual const RendererID& GetRendererID() const = 0;
        virtual ShaderReflection GetReflection() const = 0;

        virtual void SetUniform(const std::string& name, const UniformValue& value, uint32_t count) const = 0;
        void SetUniform(const std::string& name, const UniformValue& value) const { SetUniform(name, value, 0); }
        virtual void ClearCache() = 0;
        virtual void Compile() = 0;
        virtual void AsyncCompile() = 0;

        static Ref<Shader> Create(const std::string& name, std::vector<std::string> filePaths);
    protected:
        ShaderReflection m_Reflection;
    };

    class ShaderLibrary : public RefCounted
    {
    public:
        ShaderLibrary() = default;
        ~ShaderLibrary() override = default;

		void Load(std::string_view name, std::vector<std::string> filePaths);
		void Recompile();

		const Ref<Shader>& Get(const std::string& name) const;

        static Ref<ShaderLibrary> Create();
    private:
        Thread m_Thread = Thread("ShaderLibrary");
        bool m_IsCompiling = false;
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
    };
}