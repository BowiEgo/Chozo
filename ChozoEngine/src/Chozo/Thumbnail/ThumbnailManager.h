#pragma once

#include "ThumbnailRenderer.h"

#include "Chozo/Asset/Asset.h"
#include "Chozo/Core/Application.h"

namespace Chozo {
    
    class ThumbnailManager : public RefCounted
    {
    public:
        ThumbnailManager();
        ~ThumbnailManager();

        void ImportThumbnail(AssetHandle id);
        void CreateThumbnail(AssetMetadata metadata, Ref<Asset> asset);
        void RemoveThumbnail(AssetHandle id);

        Ref<Texture2D> GetThumbnail(AssetHandle assetHandle);
    private:
        // TODO: ThumbnailPool
		std::unordered_map<AssetHandle, Ref<Texture2D>> m_Thumbnails;
    };
} // namespace Chozo
