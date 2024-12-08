#include "Shader.h"

#include "RenderCommand.h"
#include "Chozo/Core/Application.h"
#include "Chozo/Renderer/Backend/OpenGL/OpenGLShader.h"

#include <GLFW/glfw3.h>

namespace Chozo {

    Ref<Shader> Shader::Create(const std::string& name, const std::vector<std::string> filePaths)
    {
        switch (RenderCommand::GetType())
        {
            case RenderAPI::Type::None:     CZ_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
            case RenderAPI::Type::OpenGL:   return Ref<OpenGLShader>::Create(name, filePaths);
        }

        CZ_CORE_ASSERT(false, "Unknown RenderAPI!");
        return nullptr;
    }

    void ShaderLibrary::Load(const std::string_view name, const std::vector<std::string> filePaths)
    {
        Ref<Shader> shader = Shader::Create(std::string(name), filePaths);
        m_Shaders.emplace(name, shader);
    }

    void ShaderLibrary::Recompile()
    {
        if (m_IsCompiling) {
            CZ_CORE_ERROR("Shaders are already compiling. Skipping new request.");
            return;
        }

        m_IsCompiling = true;
        m_Thread.Join();
        m_Thread.Dispatch([this]() {
            for (auto& [name, shader] : m_Shaders)
            {
                // Clear caches
                shader->ClearCache();
                // Compile shaders
                shader->AsyncCompile();
            }
            m_IsCompiling = false;
        });
    }

    const Ref<Shader>& ShaderLibrary::Get(const std::string &name) const
    {
		CZ_CORE_ASSERT(m_Shaders.find(name) != m_Shaders.end(), "");
        return m_Shaders.at(name);
    }

    Ref<ShaderLibrary> ShaderLibrary::Create()
    {
        return Ref<ShaderLibrary>::Create();
    }
}