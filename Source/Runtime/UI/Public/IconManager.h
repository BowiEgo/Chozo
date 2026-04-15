#pragma once

#include "CoreMinimal.h"
#include "FileUtils.h"
#include "Texture.h"
#include "ThreadPool.h"
#include "UIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogIconManager, Info);

class UI_API CIconManager {
    struct FPendingDeletion {
        TRef<CTexture> Texture;
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
    TRef<CTexture> GetOrLoadSVGIcon(const std::string& name);
    TRef<CTexture> GetOrLoadFileIcon(const std::filesystem::path& path);
    TRef<CTexture> GetDefaultIcon(const std::filesystem::path& path);

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

    std::unordered_map<std::string, TRef<CTexture>> m_SVGIconCaches;

    TRef<CTexture> m_DefaultFolderIcon, m_DefaultFileIcon;
    std::vector<FRawFileImage> m_RawFileIconCaches;
    std::unordered_map<int, TRef<CTexture>> m_FileIconCaches;
    std::unordered_map<std::string, int> m_FileIconIndices;
};
