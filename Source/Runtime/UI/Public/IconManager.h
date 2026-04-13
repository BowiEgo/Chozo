#pragma once

#include "CoreMinimal.h"
#include "FileUtils.h"
#include "RHITexture2D.h"
#include "ThreadPool.h"
#include "UIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogIconManager, Info);

class UI_API CIconManager {
    struct FPendingDeletion {
        TRef<IRHITexture2D> Texture;
        uint32 FrameIndex;
    };

public:
    static inline CIconManager& Get() {
        static CIconManager ret;
        return ret;
    }

    CIconManager();
    ~CIconManager();

private:
    void Init();
    void UpdateDeletionQueue();

public:
    TRef<IRHITexture2D> GetOrLoadSVGIcon(const std::string& name);
    TRef<IRHITexture2D> GetOrLoadFileIcon(const std::filesystem::path& path);
    TRef<IRHITexture2D> GetDefaultIcon(const std::filesystem::path& path);

    void Update();
    void ProcessRawIcons(uint32 frameIndex);
    void StopLoading();
    void RestartLoading();
    void ClearCaches();
    void Shutdown();

private:
    CThreadPool m_ThreadPool{ 4 };
    std::mutex m_ThreadMutex;

    uint32 m_CurrentFrame;

    std::vector<FPendingDeletion> m_DeletionQueue;

    std::unordered_map<std::string, TRef<IRHITexture2D>> m_SVGIconCaches;

    TRef<IRHITexture2D> m_DefaultFolderIcon, m_DefaultFileIcon;
    std::vector<FRawFileImage> m_RawFileIconCaches;
    std::unordered_map<int, TRef<IRHITexture2D>> m_FileIconCaches;
    std::unordered_map<std::string, int> m_FileIconIndices;
};
