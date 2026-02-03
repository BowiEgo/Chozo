#include "ShaderManager.h"

namespace Chozo {

DEFINE_LOG_CATEGORY(LogShaderManager);

Chozo::ShaderManager *ShaderManager::s_Instance = nullptr;

void ShaderManager::Init(GraphicsContext *context) {
    if (!s_Instance) {
        CZ_LOG(LogShaderManager, Trace, "ShaderManager Initializing...");
        s_Instance = new ShaderManager();
        s_Instance->m_Context = context;
        CZ_LOG(LogShaderManager, Info, "ShaderManager Initialized");
    }

    if (!s_Instance->m_Compiler) {
        s_Instance->m_Compiler = ShaderCompiler::Create();
    }
}

Ref<Shader> ShaderManager::Load(const ShaderCreateInfo &rep) {
    CZ_LOG(LogShaderManager, Trace, "Loading Shader: {}", rep.Name);

    ShaderID id = rep.GetHash();

    if (m_ShaderCache.contains(id)) {
        return m_ShaderCache[id];
    }

    ShaderCompilerOutput vsOutput, fsOutput;
    bool success = m_Compiler->Compile(rep, vsOutput, fsOutput);

    if (success) {
        auto shader = Shader::Create(m_Context, rep, vsOutput, fsOutput);
        m_ShaderCache[id] = shader;
        CZ_LOG(LogShaderManager, Info, "Shader: {} Loaded", rep.Name);
        return shader;
    }

    CZ_LOG(LogShaderManager, Error, "Failed to load Shader: {0}", rep.Name);
    return nullptr;
}

} // namespace Chozo
