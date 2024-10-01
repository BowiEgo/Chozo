#pragma once

#include "ThumbnailRenderer.h"

#include "Chozo/Asset/Asset.h"

namespace Chozo {
    
    class ThumbnailManager : public RefCounted
    {
    public:
        ThumbnailManager();
        ~ThumbnailManager();

        void ImportThumbnail(UUID id);
        void CreateThumbnail(AssetMetadata metadata, Ref<Asset> asset);

        Ref<Texture2D> GetThumbnail(AssetHandle assetHandle);
    private:
        // TODO: ThumbnailPool
		std::unordered_map<UUID, Ref<Texture2D>> m_Thumbnails;
    };
} // namespace Chozo
