#include "ThumbnailExporter.h"

namespace Chozo {

	std::unordered_map<AssetType, Scope<ThumbnailRenderer>> ThumbnailExporter::s_Renderers;

    void ThumbnailExporter::Init()
    {
        s_Renderers.clear();
        for (int i = 0; i < static_cast<int>(AssetType::None); i++)
        {
            AssetType type = static_cast<AssetType>(i);
            switch (type)
            {
            case AssetType::Texture:
                s_Renderers[type] = CreateScope<TextureThumbnailRenderer>(); break;
            case AssetType::Material:
                s_Renderers[type] = CreateScope<MaterialThumbnailRenderer>(); break;
            default:
                break;
            }
        }
    }

    Ref<Texture2D> ThumbnailExporter::Render(AssetMetadata metadata, Ref<Asset> asset)
    {
        return s_Renderers[asset->GetAssetType()]->Render(metadata, asset);
    }

} // namespace Chozo
