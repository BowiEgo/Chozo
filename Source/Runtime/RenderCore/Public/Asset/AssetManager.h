#pragma once

#include "CoreMinimal.h"
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
    static inline CAssetManager& Get() {
        static CAssetManager ret;
        return ret;
    }

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

private:
    CThreadPool m_ThreadPool{ 4 };
    std::mutex m_ThreadMutex;

    std::vector<FPendingDeletion> m_DeletionQueue;

    std::unordered_map<std::string, TRef<IAsset>> m_TextureCaches;
    std::unordered_map<FAssetHandle, TRef<IAsset>> m_ShaderCaches;

    TScope<CShaderCompiler> m_ShaderCompiler;
};
