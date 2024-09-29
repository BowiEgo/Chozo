#include "AssetImporter.h"

namespace Chozo
{

	std::unordered_map<AssetType, Scope<AssetSerializer>> AssetImporter::s_Serializers;

    void AssetImporter::Init()
    {
		s_Serializers.clear();
		s_Serializers[AssetType::Texture] = CreateScope<TextureSerializer>();
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
