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
		virtual void AddMemoryOnlyAsset(Ref<Asset> asset) override;
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
		const AssetMetadata& GetMetadata(const std::filesystem::path& filepath);

		AssetHandle ImportAsset(const std::filesystem::path& filepath);
		void SaveAssets();

		AssetType GetAssetTypeFromExtension(const std::string& extension);
		AssetType GetAssetTypeFromPath(const std::filesystem::path& path);

		std::filesystem::path GetFileSystemPath(const AssetMetadata& metadata);
		std::string GetFileSystemPathString(const AssetMetadata& metadata);
		std::filesystem::path GetRelativePath(const std::filesystem::path& filepath);

		template<typename T, typename... Args>
		Ref<T> CreateNewAsset(const std::string& filename, const std::string& directoryPath, Args&&... args)
		{
			AssetMetadata metadata;
			metadata.Handle = AssetHandle();
			if (directoryPath.empty() || directoryPath == ".")
				metadata.FilePath = filename;
			else
				metadata.FilePath = directoryPath + "/" + filename;
			metadata.IsDataLoaded = true;
			metadata.Type = T::GetStaticType();

			Ref<Asset> asset = T::Create(std::forward<Args>(args)...);
			asset->Handle = metadata.Handle;
			m_MemoryAssets[asset->Handle] = asset;
			metadata.FileSize = AssetImporter::Serialize(metadata, asset);

			m_AssetRegistry[metadata.Handle] = metadata;

			return asset;
		}
    private:
		void LoadAssetRegistry();
		void ProcessDirectory(const std::filesystem::path& directoryPath);
		void ReloadAssets();
		void WriteRegistryToFile();

		AssetMetadata& GetMetadataInternal(AssetHandle handle);
    private:
		std::unordered_map<AssetHandle, Ref<Asset>> m_LoadedAssets;
		std::unordered_map<AssetHandle, Ref<Asset>> m_MemoryAssets;
		AssetRegistry m_AssetRegistry;
    };
}
