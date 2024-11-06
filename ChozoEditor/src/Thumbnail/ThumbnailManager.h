#pragma once

#include "ThumbnailRenderer.h"
#include "Chozo/Asset/Asset.h"

namespace Chozo {
    
    class ThumbnailManager : public RefCounted
    {
    public:
        ThumbnailManager() {};
        ~ThumbnailManager() {};

        static void Init();

        static void ImportThumbnail(AssetHandle handle);
        static void DeleteThumbnail(AssetHandle handle);
        static void ClearUselessCaches();
        inline static void SetThumbnail(AssetHandle assetHandle, Ref<Texture2D> texture) { s_Instance->m_Thumbnails[assetHandle] = texture; }
        static Ref<Texture2D> GetThumbnail(AssetHandle assetHandle);
    private:
        static Ref<ThumbnailManager> s_Instance;

		std::unordered_map<AssetHandle, Ref<Texture2D>> m_Thumbnails;
    };
} // namespace Chozo
