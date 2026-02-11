#include "ShaderManager.h"

DEFINE_LOG_CATEGORY(LogShaderManager);

CShaderManager* CShaderManager::s_Instance = nullptr;

void CShaderManager::Init(const TRef<IRHIDevice> device) {
    if (!s_Instance) {
        CZ_LOG(LogShaderManager, Trace, "ShaderManager Initializing...");
        s_Instance = new CShaderManager();
        s_Instance->m_Device = WeakRef(device);
        CZ_LOG(LogShaderManager, Info, "ShaderManager Initialized");
    }

    if (!s_Instance->m_Compiler) {
        s_Instance->m_Compiler = CreateScope<CShaderCompiler>();
    }
}

TRef<CShader> CShaderManager::Load(const FShaderCreateInfo& info) {
    CZ_LOG(LogShaderManager, Trace, "Loading Shader: {}", info.Name);

    FShaderID id = info.GenHash();

    // if (m_ShaderCache.contains(id)) {
    //     return m_ShaderCache[id];
    // }

    FShaderCompilerOutput output;

    if (bool success = m_Compiler->Compile(info, output)) {
        auto shader = CreateRef<CShader>(info, output, m_Device);
        if (shader) {
            // m_ShaderCache[id] = shader;
            CZ_LOG(LogShaderManager, Info, "Shader: {} Loaded", info.Name);
            return shader;
        }
    }

    CZ_LOG(LogShaderManager, Error, "Failed to load Shader: {0}", info.Name);
    return nullptr;
}
