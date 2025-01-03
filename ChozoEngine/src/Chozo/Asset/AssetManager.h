#pragma once

#include "czpch.h"

#include "Asset.h"

namespace Chozo {

	class AssetManager : public RefCounted
	{
	public:
		AssetManager() = default;
		~AssetManager() override = default;

		virtual Ref<Asset> GetAsset(AssetHandle assetHandle) = 0;
		virtual std::vector<AssetMetadata> GetAssetsModified() = 0;
		virtual AssetHandle AddMemoryOnlyAsset(Ref<Asset> asset) = 0;
		virtual bool ReloadData(AssetHandle assetHandle) = 0;
		virtual bool IsAssetHandleValid(AssetHandle assetHandle) = 0;
		virtual bool IsMemoryAsset(AssetHandle handle) = 0;
		virtual bool IsAssetLoaded(AssetHandle handle) = 0;
		virtual void RemoveAsset(AssetHandle handle) = 0;

		virtual std::unordered_set<AssetHandle> GetAllAssetsWithType(AssetType type) = 0;
		virtual const std::unordered_map<AssetHandle, Ref<Asset>>& GetLoadedAssets() = 0;
		virtual const std::unordered_map<AssetHandle, Ref<Asset>>& GetMemoryOnlyAssets() = 0;
	};
}
