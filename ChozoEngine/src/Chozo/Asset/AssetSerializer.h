#pragma once

#include "Asset.h"

namespace Chozo {

    class AssetSerializer
    {
    public:
		virtual ~AssetSerializer() = default;

		virtual uint64_t Serialize(const AssetMetadata& metadata, Ref<Asset>& asset) const = 0;
		virtual Ref<Asset> Deserialize(const AssetMetadata& metadata) const = 0;
    };

    class TextureSerializer : public AssetSerializer
	{
	public:
		virtual uint64_t Serialize(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
		virtual Ref<Asset> Deserialize(const AssetMetadata& metadata) const override;
	};
}
