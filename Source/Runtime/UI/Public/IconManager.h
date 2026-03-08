#pragma once

#include "CoreMinimal.h"
#include "FileUtils.h"
#include "RHIContext.h"
#include "RHITexture2D.h"
#include "UIExport.h"

DECLARE_LOG_CATEGORY_EXTERN(LogIconManager, Info);

class UI_API CIconManager {
public:
    static inline CIconManager& Get(IRHIContext* context) {
        static CIconManager ret(context);
        return ret;
    }

    CIconManager(IRHIContext* context);
    ~CIconManager();

    TRef<IRHITexture2D> GetOrLoadSVGIcon(const std::string& name);
    TRef<IRHITexture2D> GetOrLoadFileIcon(const std::filesystem::path& path);
    FRawFileImage GetDefaultIconImage(const std::filesystem::path& path);

    void Shutdown();

private:
    IRHIContext* m_GraphicContext;

    std::unordered_map<std::string, TRef<IRHITexture2D>> m_SVGIconCaches;
    std::unordered_map<int, TRef<IRHITexture2D>> m_FileIconCaches;
    std::unordered_map<std::string, int> m_FileIconIndices;
};
