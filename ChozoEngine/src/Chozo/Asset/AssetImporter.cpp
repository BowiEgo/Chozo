#include "AssetImporter.h"

namespace Chozo
{

	std::unordered_map<AssetType, Scope<AssetSerializer>> AssetImporter::s_Serializers;

    void AssetImporter::Init()
    {
		s_Serializers.clear();
        for (int i = 0; i < static_cast<int>(AssetType::None); i++)
        {
            AssetType type = static_cast<AssetType>(i);
            switch (type)
            {
            case AssetType::Texture:
                s_Serializers[type] = CreateScope<TextureSerializer>(); break;
            default:
                break;
            }
        }
    }

    uint64_t AssetImporter::Serialize(const AssetMetadata& metadata, Ref<Asset>& asset)
    {
		return s_Serializers[metadata.Type]->Serialize(metadata, asset);
    }

    Ref<Asset> AssetImporter::Deserialize(const AssetMetadata &metadata)
    {
		return s_Serializers[metadata.Type]->Deserialize(metadata);
    }
}
