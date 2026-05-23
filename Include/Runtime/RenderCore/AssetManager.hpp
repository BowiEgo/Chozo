#pragma once

#include <Runtime/RHI/RHITypes.hpp>
#include <Runtime/RenderCore/Asset.hpp>
#include <Runtime/RenderCore/Shader.hpp>
#include <Runtime/RenderCore/ShaderCompiler.hpp>

namespace CZ {

class AssetManager {
public:
    static AssetManager& Get();

    AssetManager();
    ~AssetManager() {};

    void ClearCaches();
    void Shutdown();

    Shader GetOrLoadShader(const ShaderSpecification& spec);

private:
    std::unordered_map<AssetHandle, Shader> m_ShaderCaches;
};
} // namespace CZ
