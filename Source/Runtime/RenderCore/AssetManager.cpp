#include <Runtime/RenderCore/AssetManager.hpp>

#include <Core/Log/LogMacros.hpp>

namespace CZ {

AssetManager& AssetManager::Get() {
    static AssetManager instance;
    return instance;
}

AssetManager::AssetManager() {
    // m_CheckboardTexture = GetOrLoadTexture("textures://CheckerboardTexture.png");
}

void AssetManager::ClearCaches() {
    // m_Caches.clear();
    // m_TextureCaches.clear();
}

void AssetManager::Shutdown() {
    ClearCaches();
    // m_CheckboardTexture.Reset();
}

Shader AssetManager::GetOrLoadShader(const ShaderSpecification& spec) {
    // CZ_LOG(LogAssetManager, Trace, "Loading Shader: {}", spec.Name);

    // auto it = m_Caches.find(specs);
    // if (it != m_Caches.end()) {
    //     return it->second;
    // }

    CZ_CORE_LOG(Trace, "Compiling Shader: {}", spec.Name);

    ShaderCompilerMultiInput input;
    input.VirtualPath = spec.VirtualPath;
    input.Stages      = spec.Stages;
    input.Macros.Add(spec.Definitions);

    std::unordered_map<ShaderStage, ShaderCompilerOutput> outputs;

    if (bool success = ShaderCompiler::Get().Compile(input, outputs)) {
        auto shader        = Shader::Create(spec, outputs);
        AssetHandle handle = AssetHandle::Generate();

        shader.SetHandle(handle);
        m_ShaderCaches[handle] = shader;

        return shader;
    }

    return Shader(); // Return an empty/invalid shader on failure
}

} // namespace CZ
