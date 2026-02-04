#include "ShaderManager.h"

DEFINE_LOG_CATEGORY(LogShaderManager);

CShaderManager *CShaderManager::s_Instance = nullptr;

void CShaderManager::Init(CGraphicsContext *context) {
    if (!s_Instance) {
        CZ_LOG(LogShaderManager, Trace, "ShaderManager Initializing...");
        s_Instance = new CShaderManager();
        s_Instance->m_Context = context;
        CZ_LOG(LogShaderManager, Info, "ShaderManager Initialized");
    }

    if (!s_Instance->m_Compiler) {
        s_Instance->m_Compiler = CShaderCompiler::Create();
    }
}

TRef<CShader> CShaderManager::Load(const FShaderCreateInfo &rep) {
    CZ_LOG(LogShaderManager, Trace, "Loading Shader: {}", rep.Name);

    FShaderID id = rep.GetHash();

    if (m_ShaderCache.contains(id)) {
        return m_ShaderCache[id];
    }

    FShaderCompilerOutput vsOutput, fsOutput;
    bool success = m_Compiler->Compile(rep, vsOutput, fsOutput);

    if (success) {
        auto shader = CShader::Create(m_Context, rep, vsOutput, fsOutput);
        m_ShaderCache[id] = shader;
        CZ_LOG(LogShaderManager, Info, "Shader: {} Loaded", rep.Name);
        return shader;
    }

    CZ_LOG(LogShaderManager, Error, "Failed to load Shader: {0}", rep.Name);
    return nullptr;
}
