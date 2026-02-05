#include "ShaderManager.h"

DEFINE_LOG_CATEGORY(LogShaderManager);

CShaderManager* CShaderManager::s_Instance = nullptr;

void CShaderManager::Init(const TRef<IRHIDevice> device) {
    if (!s_Instance) {
        CZ_LOG(LogShaderManager, Trace, "ShaderManager Initializing...");
        s_Instance = new CShaderManager();
        s_Instance->m_Device = device;
        CZ_LOG(LogShaderManager, Info, "ShaderManager Initialized");
    }

    if (!s_Instance->m_Compiler) {
        s_Instance->m_Compiler = CreateScope<CShaderCompiler>();
    }
}

TRef<CShader> CShaderManager::Load(const FShaderCreateInfo& rep) {
    CZ_LOG(LogShaderManager, Trace, "Loading Shader: {}", rep.Name);

    FShaderID id = rep.GenHash();

    if (m_ShaderCache.contains(id)) {
        return m_ShaderCache[id];
    }

    FShaderCompiledData compiledData;
    compiledData.ID = id;
    compiledData.Name = rep.Name;

    FShaderCompilerOutput vsOutput, fsOutput;
    bool success = m_Compiler->Compile(rep, vsOutput,
                                       fsOutput); // TODO: make more flexable

    compiledData[EShaderStage::Vertex] = vsOutput;
    compiledData[EShaderStage::Fragment] = fsOutput;

    if (success) {
        auto shader = CreateRef<CShader>(m_Device, compiledData);
        if (shader) {
            m_ShaderCache[id] = shader;
            CZ_LOG(LogShaderManager, Info, "Shader: {} Loaded", rep.Name);
            return shader;
        }
    }

    CZ_LOG(LogShaderManager, Error, "Failed to load Shader: {0}", rep.Name);
    return nullptr;
}
