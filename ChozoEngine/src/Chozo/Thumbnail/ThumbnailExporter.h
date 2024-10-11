#pragma once

#include "ThumbnailRenderer.h"

#include "Chozo/Asset/Asset.h"

namespace Chozo {
    
    class ThumbnailExporter
    {
    public:
        static void Init();

        static Ref<Texture2D> Render(AssetMetadata metadata, Ref<Asset> asset);

		static MaterialThumbnailRenderer* GetMaterialThumbnailRenderer() { return dynamic_cast<MaterialThumbnailRenderer*>(s_Renderers[AssetType::Material].get()); }
    private:
		static std::unordered_map<AssetType, Scope<ThumbnailRenderer>> s_Renderers;
    };
    
} // namespace Chozo