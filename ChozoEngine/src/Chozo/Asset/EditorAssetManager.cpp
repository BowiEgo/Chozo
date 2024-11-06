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

        if (metadata.IsDataLoaded)
            return m_LoadedAssets[assetHandle];

        auto handle = LoadAsset(metadata);
        if (handle.isValid())
            return GetAsset(handle);
        else
            return nullptr;
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

        m_AssetRegistry.Remove(handle);
        m_LoadedAssets.erase(handle);
        m_MemoryAssets.erase(handle);

        WriteRegistryToFile();

        fs::path filePath = Utils::File::GetAssetDirectory() / metadata.FilePath;
        Utils::File::DeleteFile(filePath.string() + ".asset");
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

    AssetHandle EditorAssetManager::LoadAsset(const fs::path &filepath)
    {
		fs::path path = GetRelativePath(filepath);
        if (path.extension().string() != ".asset")
            return 0;

        path = path.parent_path() / path.stem();

        AssetMetadata metadata = GetMetadata(filepath);
        // Create new metadata if none exists
        if (!metadata.IsValid())
            metadata.FilePath = path;

        if (metadata.IsDataLoaded)
            return metadata.Handle;

        return LoadAsset(metadata);
    }

    AssetHandle EditorAssetManager::LoadAsset(AssetMetadata metadata)
    {
        CZ_CORE_INFO("Loading asset {}", metadata.FilePath.string());

        // Deserialize and store asset if successful
        auto asset = AssetImporter::Deserialize(metadata);
        metadata.IsDataLoaded = static_cast<bool>(asset);
        if (asset)
        {
            asset->Handle = metadata.Handle;
            m_LoadedAssets[metadata.Handle] = asset;
            m_AssetRegistry[metadata.Handle] = metadata;
            return metadata.Handle;
        }
        return 0;
    }

    void EditorAssetManager::SaveAssets()
    {
        for (auto [handle, metadata] : m_AssetRegistry)
        {
            auto asset = GetAsset(handle);
            AssetImporter::Serialize(metadata, asset);
        }
        WriteRegistryToFile();
    }

    void EditorAssetManager::SaveAsset(Ref<Asset> asset, const fs::path &filepath)
    {
		fs::path path = GetRelativePath(filepath);

        AssetMetadata metadata;

        if (asset->Handle != 0)
        {
            metadata = GetMetadata(asset->Handle);
            m_MemoryAssets.erase(asset->Handle);
        }
        else
        {
			metadata.Handle = AssetHandle();
			metadata.Type = asset->GetAssetType();
            asset->Handle = metadata.Handle;
        }

        metadata.FilePath = path;
        metadata.IsDataLoaded = true;
        metadata.IsMemoryAsset = false;
        metadata.FileSize = AssetImporter::Serialize(metadata, asset);

        m_LoadedAssets[asset->Handle] = asset;
        m_AssetRegistry[metadata.Handle] = metadata;

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
            fs::path path(metadata.FilePath.string() + ".asset");
            fs::path filepath = Utils::File::GetAssetDirectory() / path;
            // // TODO: Remove
            // auto handleLoaded = LoadAsset(filepath);
            // if (handle == 0 || handleLoaded == 0)
            // {
            //     RemoveAsset(handle);
            // }
        }
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
        serializer.Serialize("../assets/AssetRegistry.czar");
    }

    AssetMetadata &EditorAssetManager::GetMetadataInternal(AssetHandle handle)
    {
        if (m_AssetRegistry.Contains(handle))
			return m_AssetRegistry[handle];

		return s_NullMetadata;
    }
}