#include "EditorAssetManager.h"

#include "AssetImporter.h"
#include "AssetRegistrySerializer.h"

#include "Chozo/Utilities/PlatformUtils.h"
#include "Chozo/Utilities/StringUtils.h"
#include "Chozo/Project/Project.h"

#include "Chozo/Renderer/Texture.h"

namespace Chozo {

	static AssetMetadata s_NullMetadata;

    EditorAssetManager::EditorAssetManager()
    {
        AssetImporter::Init();
    }

    EditorAssetManager::~EditorAssetManager()
    {
    }

    Ref<Asset> EditorAssetManager::GetAsset(AssetHandle assetHandle)
    {
        if (IsMemoryAsset(assetHandle))
			return m_MemoryAssets[assetHandle];

        auto& metadata = GetMetadataInternal(assetHandle);
        if (!metadata.IsValid())
            return nullptr;

        if (!metadata.IsDataLoaded)
        {
            auto asset = AssetImporter::TryLoadData(metadata);
            metadata.IsDataLoaded = !!asset;
            if (asset)
            {
                asset->Handle = assetHandle;
                m_LoadedAssets[assetHandle] = asset;
            }
            return asset;
        }
        else
            return m_LoadedAssets[assetHandle];
    }

    AssetHandle EditorAssetManager::AddMemoryOnlyAsset(Ref<Asset> asset)
    {
        asset->Handle = AssetHandle();
        AssetMetadata metadata;
        metadata.Handle = asset->Handle;
        metadata.Type = asset->GetAssetType();
        metadata.IsMemoryAsset = true;
        m_AssetRegistry[metadata.Handle] = metadata;

        m_MemoryAssets[asset->Handle] = asset;

        return asset->Handle;
    }

    bool EditorAssetManager::ReloadData(AssetHandle handle)
    {
        return false;
    }

    bool EditorAssetManager::IsAssetHandleValid(AssetHandle handle)
    {
        AssetMetadata metadata = GetMetadata(handle);
        return metadata.IsValid();
    }

    bool EditorAssetManager::IsAssetLoaded(AssetHandle handle)
    {
        return false;
    }

    void EditorAssetManager::RemoveAsset(AssetHandle handle)
    {
        AssetMetadata metadata = GetMetadata(handle);
        if (metadata.IsValid())
        {
            m_AssetRegistry.Remove(handle);
            SaveAssets();

            fs::path filePath = Utils::File::GetAssetDirectory() / metadata.FilePath;
            Utils::File::DeleteFile(filePath.string() + ".asset");
        }
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

    const AssetMetadata &EditorAssetManager::GetMetadata(const fs::path &filepath)
    {
		const auto relativePath = GetRelativePath(filepath).replace_extension().string();
        
        for (auto& [handle, metadata] : m_AssetRegistry)
		{
			if (metadata.FilePath == relativePath)
				return metadata;
		}

		return s_NullMetadata;
    }

    AssetHandle EditorAssetManager::ImportAsset(const fs::path &filepath)
    {
		fs::path path = GetRelativePath(filepath);

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

    AssetType EditorAssetManager::GetAssetTypeFromPath(const fs::path &path)
    {
		return GetAssetTypeFromExtension(path.extension().string());
    }

    fs::path EditorAssetManager::GetFileSystemPath(const AssetMetadata &metadata)
    {
		return Project::GetAssetDirectory() / metadata.FilePath;
    }

    std::string EditorAssetManager::GetFileSystemPathString(const AssetMetadata &metadata)
    {
		return GetFileSystemPath(metadata).string();
    }

    fs::path EditorAssetManager::GetRelativePath(const fs::path &filepath)
    {
        return fs::relative(filepath, Utils::File::GetAssetDirectory());
    }

    void EditorAssetManager::LoadAssetRegistry()
    {
        AssetRegistrySerializer serializer(m_AssetRegistry);

        m_AssetRegistry = serializer.Deserialize("../assets/AssetRegistry.czar");

        for (auto& [handle, metadata] : m_AssetRegistry)
        {
            fs::path path(metadata.FilePath);
            fs::path filepath = Utils::File::GetAssetDirectory() / path;
            
            // TODO: Remove
		    Ref<Asset> asset = AssetImporter::Deserialize(metadata);
            if (asset)
            {
			    asset->Handle = handle;
			    m_LoadedAssets[handle] = asset;
            }
        }
        return;
    }

    void EditorAssetManager::ProcessDirectory(const fs::path &directoryPath)
    {
        for (auto entry : fs::directory_iterator(directoryPath))
		{
			if (entry.is_directory())
				ProcessDirectory(entry.path());
			else
				ImportAsset(entry.path());
		}
    }

    void EditorAssetManager::ReloadAssets()
    {
		ProcessDirectory(Utils::File::GetAssetDirectory());
    }

    void EditorAssetManager::WriteRegistryToFile()
    {
        AssetRegistrySerializer serializer(m_AssetRegistry);

        for (auto [handle, metadata] : m_AssetRegistry)
        {
            auto asset = GetAsset(handle);
            AssetImporter::Serialize(metadata, asset);
        }

        serializer.Serialize("../assets/AssetRegistry.czar");
    }

    AssetMetadata &EditorAssetManager::GetMetadataInternal(AssetHandle handle)
    {
        if (m_AssetRegistry.Contains(handle))
			return m_AssetRegistry[handle];

		return s_NullMetadata;
    }
}