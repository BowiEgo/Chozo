#pragma once

#include "CoreMinimal.h"
#include "Material.h"
#include "RenderCoreExport.h"
#include "Shader.h"
#include "ShaderCompiler.h"
#include "Texture.h"
#include "ThreadPool.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAssetManager, Info);

class RENDER_CORE_API CAssetManager {
    struct FPendingDeletion {
        TRef<IAsset> Asset;
        uint32 FrameIndex;
    };

public:
    static CAssetManager& Get();

    CAssetManager();
    ~CAssetManager() {};

private:
    // void Init();

public:
    // void StopLoading();
    // void RestartLoading();
    void ClearCaches();
    void Shutdown();

    TRef<CTexture> GetOrLoadTexture(const std::string& virtualPath);
    TRef<CShader> GetOrLoadShader(const FShaderSpecification& spec);
    TRef<CMaterial> GetOrLoadMaterial(const FMaterialSpecification& spec);

    const TRef<CTexture> GetCheckboardTexture() const { return m_CheckboardTexture; }

    TRef<IAsset> GetAsset(const FAssetHandle& handle) const {
        auto it = m_Caches.find(handle);
        return it != m_Caches.end() ? it->second : nullptr;
    }

private:
    CThreadPool m_ThreadPool{ 4 };
    std::mutex m_ThreadMutex;

    std::vector<FPendingDeletion> m_DeletionQueue;

    std::unordered_map<std::string, TRef<IAsset>> m_TextureCaches;
    std::unordered_map<FAssetHandle, TRef<IAsset>> m_Caches;

    TScope<CShaderCompiler> m_ShaderCompiler;
    TRef<CTexture> m_CheckboardTexture;
};
