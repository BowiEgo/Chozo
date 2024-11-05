#pragma once

#include "Asset.h"
#include "AssetSerializer.h"

namespace Chozo
{

    class AssetImporter
    {
    public:
        static void Init();
		static uint64_t Serialize(const AssetMetadata& metadata, Ref<Asset>& asset);
		static Ref<Asset> Deserialize(AssetMetadata& metadata);
    private:
		static std::unordered_map<AssetType, Scope<AssetSerializer>> s_Serializers;
    };
}
