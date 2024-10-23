#pragma once

#include "Asset.h"
#include "Chozo/Scene/Scene.h"
#include "Chozo/Renderer/Material.h"

namespace Chozo {

    class AssetSerializer
    {
    public:
		virtual ~AssetSerializer() = default;

		virtual uint64_t Serialize(const AssetMetadata& metadata, Ref<Asset>& asset) const = 0;
		virtual Ref<Asset> Deserialize(const AssetMetadata& metadata) const = 0;
    };

	class SceneSerializer : public AssetSerializer
	{
	public:
		virtual uint64_t Serialize(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
		virtual Ref<Asset> Deserialize(const AssetMetadata& metadata) const override;
	private:
		std::string SerializeToYAML(Ref<Scene> scene) const;
		Ref<Scene> DeserializeFromYAML(const std::string& yamlString) const;
	};

    class TextureSerializer : public AssetSerializer
	{
	public:
		virtual uint64_t Serialize(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
		virtual Ref<Asset> Deserialize(const AssetMetadata& metadata) const override;
	};

	class MaterialSerializer : public AssetSerializer
	{
	public:
		virtual uint64_t Serialize(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
		virtual Ref<Asset> Deserialize(const AssetMetadata& metadata) const override;
	private:
		std::string SerializeToYAML(Ref<Material> material) const;
		Ref<Material> DeserializeFromYAML(const std::string& yamlString) const;
	};
}
