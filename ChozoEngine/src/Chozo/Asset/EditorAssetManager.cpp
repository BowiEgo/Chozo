#include "EditorAssetManager.h"

#include "AssetExtensions.h"
#include "AssetImporter.h"
#include "AssetRegistrySerializer.h"

#include "Chozo/Utilities/PlatformUtils.h"
#include "Chozo/Utilities/StringUtils.h"
#include "Chozo/Project/Project.h"

#include "Chozo/Renderer/Texture.h"

namespace Chozo {

    extern const std::filesystem::path g_AssetsPath;
	static AssetMetadata s_NullMetadata;

    EditorAssetManager::EditorAssetManager()
    {
        AssetImporter::Init();
        LoadAssetRegistry();
    }

    EditorAssetManager::~EditorAssetManager()
    {
    }

    Ref<Asset> EditorAssetManager::GetAsset(AssetHandle assetHandle)
    {
        return m_MemoryAssets[assetHandle];

        auto& metadata = GetMetadataInternal(assetHandle);
        if (!metadata.IsValid())
            return nullptr;

        Ref<Asset> asset;
        // if (!metadata.IsDataLoaded)
        // {
        //     metadata.IsDataLoaded = AssetImporter::TryLoadData(metadata, asset);
        //     if (!metadata.IsDataLoaded)
        //         return nullptr;
            
        //     m_LoadedAssets[assetHandle] = asset;
        // }
        // else
        //     asset = m_LoadedAssets[assetHandle];

        return asset;
    }

    void EditorAssetManager::AddMemoryOnlyAsset(Ref<Asset> asset)
    {
        AssetMetadata metadata;
        metadata.Handle = asset->Handle;
        metadata.Type = asset->GetAssetType();
        metadata.IsMemoryAsset = true;
        m_AssetRegistry[metadata.Handle] = metadata;

        m_MemoryAssets[asset->Handle] = asset;
    }

    bool EditorAssetManager::ReloadData(AssetHandle handle)
    {
        return false;
    }

    bool EditorAssetManager::IsAssetHandleValid(AssetHandle handle)
    {
        return false;
    }

    bool EditorAssetManager::IsAssetLoaded(AssetHandle handle)
    {
        return false;
    }

    void EditorAssetManager::RemoveAsset(AssetHandle handle)
    {
        
    }

    std::unordered_set<AssetHandle> EditorAssetManager::GetAllAssetsWithType(AssetType type)
    {
        return std::unordered_set<AssetHandle>();
    }

    const std::unordered_map<AssetHandle, Ref<Asset>> &EditorAssetManager::GetLoadedAssets()
    {
        return m_LoadedAssets;
    }

    const std::unordered_map<AssetHandle, Ref<Asset>> &EditorAssetManager::GetMemoryOnlyAssets()
    {
        return m_MemoryAssets;
    }

    const AssetMetadata &EditorAssetManager::GetMetadata(AssetHandle handle)
    {
        if (m_AssetRegistry.Contains(handle))
			return m_AssetRegistry[handle];

		return s_NullMetadata;
    }

    const AssetMetadata &EditorAssetManager::GetMetadata(const std::filesystem::path &filepath)
    {
		const auto relativePath = GetRelativePath(filepath).replace_extension().string();
        
        for (auto& [handle, metadata] : m_AssetRegistry)
		{
			if (metadata.FilePath == relativePath)
				return metadata;
		}

		return s_NullMetadata;
    }

    AssetHandle EditorAssetManager::ImportAsset(const std::filesystem::path &filepath)
    {
		std::filesystem::path path = GetRelativePath(filepath);

        if (auto& metadata = GetMetadata(path); metadata.IsValid())
            return metadata.Handle;

        AssetType type = GetAssetTypeFromPath(path);
		if (type == AssetType::None)
			return 0;

        AssetMetadata metadata;
        metadata.Handle = AssetHandle();
        metadata.FilePath = path;
        metadata.Type = type;
        m_AssetRegistry[metadata.Handle] = metadata;

        return metadata.Handle;
    }

    void EditorAssetManager::SaveAssets()
    {
        WriteRegistryToFile();
    }

    AssetType EditorAssetManager::GetAssetTypeFromExtension(const std::string &extension)
    {
		std::string ext = Utils::String::ToLowerCopy(extension);

        if (s_AssetExtensionMap.find(ext) == s_AssetExtensionMap.end())
			return AssetType::None;

		return s_AssetExtensionMap.at(ext.c_str());
    }

    AssetType EditorAssetManager::GetAssetTypeFromPath(const std::filesystem::path &path)
    {
		return GetAssetTypeFromExtension(path.extension().string());
    }

    std::filesystem::path EditorAssetManager::GetFileSystemPath(const AssetMetadata &metadata)
    {
		return Project::GetAssetDirectory() / metadata.FilePath;
    }

    std::string EditorAssetManager::GetFileSystemPathString(const AssetMetadata &metadata)
    {
		return GetFileSystemPath(metadata).string();
    }

    std::filesystem::path EditorAssetManager::GetRelativePath(const std::filesystem::path &filepath)
    {
        return std::filesystem::relative(filepath, g_AssetsPath);
    }

    void EditorAssetManager::LoadAssetRegistry()
    {
        AssetRegistrySerializer serializer(m_AssetRegistry);

        m_AssetRegistry = serializer.Deserialize("../assets/AssetRegistry.czar");

        for (auto& [handle, metadata] : m_AssetRegistry)
        {
            std::filesystem::path path(metadata.FilePath);
            std::filesystem::path filepath = g_AssetsPath / path;
            
            // TODO: Remove
		    Ref<Asset> asset = AssetImporter::Deserialize(metadata);

			asset->Handle = handle;
			m_MemoryAssets[handle] = asset;
        }
        return;
    }

    void EditorAssetManager::ProcessDirectory(const std::filesystem::path &directoryPath)
    {
        for (auto entry : std::filesystem::directory_iterator(directoryPath))
		{
			if (entry.is_directory())
				ProcessDirectory(entry.path());
			else
				ImportAsset(entry.path());
		}
    }

    void EditorAssetManager::ReloadAssets()
    {
		ProcessDirectory(g_AssetsPath);
    }

    void EditorAssetManager::WriteRegistryToFile()
    {
        AssetRegistrySerializer serializer(m_AssetRegistry);

        serializer.Serialize("../assets/AssetRegistry.czar");
    }

    AssetMetadata &EditorAssetManager::GetMetadataInternal(AssetHandle handle)
    {
        if (m_AssetRegistry.Contains(handle))
			return m_AssetRegistry[handle];

		return s_NullMetadata;
    }
}