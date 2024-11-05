#pragma once

#include "AssetManager.h"
#include "AssetImporter.h"

namespace Chozo {

    class EditorAssetManager : public AssetManager
    {
    public:
        EditorAssetManager();
        virtual ~EditorAssetManager();

        virtual Ref<Asset> GetAsset(AssetHandle assetHandle) override;
		virtual AssetHandle AddMemoryOnlyAsset(Ref<Asset> asset) override;
		virtual bool ReloadData(AssetHandle assetHandle) override;
		virtual bool IsAssetHandleValid(AssetHandle assetHandle) override;
		virtual bool IsMemoryAsset(AssetHandle handle) override { return m_MemoryAssets.find(handle) != m_MemoryAssets.end(); }
		virtual bool IsAssetLoaded(AssetHandle handle) override;
		virtual void RemoveAsset(AssetHandle handle) override;

		virtual std::unordered_set<AssetHandle> GetAllAssetsWithType(AssetType type) override;
		virtual const std::unordered_map<AssetHandle, Ref<Asset>>& GetLoadedAssets() override;
		virtual const std::unordered_map<AssetHandle, Ref<Asset>>& GetMemoryOnlyAssets() override;

		// Editor-only
		const AssetMetadata& GetMetadata(AssetHandle handle);
		const AssetMetadata& GetMetadata(const fs::path& filepath);

		AssetHandle ImportAsset(const fs::path& filepath);
		void SaveAssets();
		void SaveAsset(Ref<Asset> asset, const fs::path &filepath);

		AssetType GetAssetTypeFromExtension(const std::string& extension);
		AssetType GetAssetTypeFromPath(const fs::path& path);

		fs::path GetFileSystemPath(const AssetMetadata& metadata);
		std::string GetFileSystemPathString(const AssetMetadata& metadata);
		fs::path GetRelativePath(const fs::path& filepath);

		template<typename T, typename... Args>
		Ref<T> CreateNewAsset(const std::string& filename, const std::string& directoryPath, Args&&... args)
		{
			fs::path filePath;

			if (directoryPath.empty() || directoryPath == ".")
				filePath = filename;
			else
				filePath = directoryPath + "/" + filename;

			Ref<Asset> asset = T::Create(std::forward<Args>(args)...);
			SaveAsset(asset, filePath);

			return asset;
		}
		
		void LoadAssetRegistry();
    private:
		void ProcessDirectory(const fs::path& directoryPath);
		void ReloadAssets();
		void WriteRegistryToFile();

		AssetMetadata& GetMetadataInternal(AssetHandle handle);
    private:
		std::unordered_map<AssetHandle, Ref<Asset>> m_LoadedAssets;
		std::unordered_map<AssetHandle, Ref<Asset>> m_MemoryAssets;
		AssetRegistry m_AssetRegistry;
    };
}
