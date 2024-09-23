#pragma once

#include "czpch.h"

#include "Asset.h"

namespace Chozo {

	class AssetManager
	{
	public:
		AssetManager() = default;
		~AssetManager() = default;

		Ref<Asset> GetAsset(AssetHandle assetHandle);
		void AddAsset(Ref<Asset> asset);
		bool ReloadData(AssetHandle assetHandle);
		bool IsAssetHandleValid(AssetHandle assetHandle);
		bool IsMemoryAsset(AssetHandle handle);
		bool IsAssetLoaded(AssetHandle handle);
		void RemoveAsset(AssetHandle handle);

		const std::unordered_map<AssetHandle, Ref<Asset>>& GetLoadedAssets();
		const std::unordered_map<AssetHandle, Ref<Asset>>& GetMemoryOnlyAssets();
    private:
		std::unordered_map<AssetHandle, Ref<Asset>> m_Assets;
	};

}
