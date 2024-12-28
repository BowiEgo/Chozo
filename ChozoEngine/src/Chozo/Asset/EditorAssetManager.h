#pragma once

#include "AssetRegistry.h"
#include "AssetManager.h"

namespace Chozo {

    class EditorAssetManager : public AssetManager
    {
    public:
        EditorAssetManager();
        ~EditorAssetManager() override;

        Ref<Asset> GetAsset(AssetHandle assetHandle) override;
    	std::vector<AssetMetadata> GetAssetsModified() override;
		AssetHandle AddMemoryOnlyAsset(Ref<Asset> asset) override;
		bool ReloadData(AssetHandle assetHandle) override;
		bool IsAssetHandleValid(AssetHandle assetHandle) override;
		bool IsMemoryAsset(AssetHandle handle) override { return m_MemoryAssets.find(handle) != m_MemoryAssets.end(); }
		bool IsAssetLoaded(AssetHandle handle) override;
		void RemoveAsset(AssetHandle handle) override;

		std::unordered_set<AssetHandle> GetAllAssetsWithType(AssetType type) override;
		const std::unordered_map<AssetHandle, Ref<Asset>>& GetLoadedAssets() override;
		const std::unordered_map<AssetHandle, Ref<Asset>>& GetMemoryOnlyAssets() override;

		// Editor-only
		const AssetMetadata& GetMetadata(AssetHandle handle);
		const AssetMetadata& GetMetadata(const fs::path& filepath);

		AssetHandle ImportAsset(const fs::path& filepath);
		AssetHandle LoadAsset(const fs::path& filepath);
		AssetHandle LoadAsset(AssetMetadata& metadata);
		void SaveAssets();
		uint64_t SaveAsset(Ref<Asset>& asset, AssetMetadata &metadata);
		void ExportAsset(Ref<Asset>& asset, const fs::path &filepath);
    	void RegisterAssetCallback(Ref<Asset>& asset);

		static AssetType GetAssetTypeFromExtension(const std::string& extension);
		static AssetType GetAssetTypeFromPath(const fs::path& path);

		static fs::path GetFileSystemPath(const AssetMetadata& metadata);
		static std::string GetFileSystemPathString(const AssetMetadata& metadata);
		static fs::path GetRelativePath(const fs::path& filepath);

		template<typename T, typename... Args>
		Ref<T> CreateNewAsset(const std::string& filename, const std::string& directoryPath, Args&&... args)
		{
			fs::path filePath;

			if (directoryPath.empty() || directoryPath == ".")
				filePath = filename;
			else
				filePath = directoryPath + "/" + filename;

			Ref<Asset> asset = T::Create(std::forward<Args>(args)...);
			ExportAsset(asset, filePath);
			RegisterAssetCallback(asset);

			return asset;
		}
		
		void LoadAssetRegistry();
    private:
		void ProcessDirectory(const fs::path& directoryPath);
		void ReloadAssets();
		void WriteRegistryToFile() const;

		AssetMetadata& GetMetadataInternal(AssetHandle handle);
    private:
		std::unordered_map<AssetHandle, Ref<Asset>> m_LoadedAssets;
		std::unordered_map<AssetHandle, Ref<Asset>> m_MemoryAssets;
		AssetRegistry m_AssetRegistry;
    };
}
