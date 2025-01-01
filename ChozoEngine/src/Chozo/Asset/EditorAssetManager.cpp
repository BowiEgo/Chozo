#include "EditorAssetManager.h"

#include "AssetImporter.h"
#include "AssetRegistrySerializer.h"

#include "Chozo/Utilities/PlatformUtils.h"
#include "Chozo/Utilities/StringUtils.h"
#include "Chozo/Project/Project.h"

namespace Chozo {

	static AssetMetadata s_NullMetadata;

    EditorAssetManager::EditorAssetManager()
    {
        AssetImporter::Init();
    }

    EditorAssetManager::~EditorAssetManager() = default;

    Ref<Asset> EditorAssetManager::GetAsset(const AssetHandle assetHandle)
    {
        if (IsMemoryAsset(assetHandle))
			return m_MemoryAssets[assetHandle];

        auto& metadata = GetMetadataInternal(assetHandle);
        if (!metadata.IsValid())
            return nullptr;

        if (metadata.IsDataLoaded)
            return m_LoadedAssets[assetHandle];

        if (const auto handle = LoadAsset(metadata); handle.isValid())
            return GetAsset(handle);

        return nullptr;
    }

    std::vector<AssetMetadata> EditorAssetManager::GetAssetsModified()
    {
        std::vector<AssetMetadata> result;
        result.reserve(m_AssetRegistry.Count());

        for (const auto& [assetHandle, metadata] : m_AssetRegistry)
        {
            if (metadata.IsValid() && !metadata.IsMemoryAsset && metadata.IsModified())
                result.push_back(metadata);
        }

        return result;
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

    bool EditorAssetManager::ReloadData(AssetHandle assetHandle)
    {
        return false;
    }

    bool EditorAssetManager::IsAssetHandleValid(const AssetHandle assetHandle)
    {
        AssetMetadata metadata = GetMetadata(assetHandle);
        return metadata.IsValid();
    }

    bool EditorAssetManager::IsAssetLoaded(AssetHandle handle)
    {
        return false;
    }

    void EditorAssetManager::RemoveAsset(AssetHandle handle)
    {
        AssetMetadata metadata = GetMetadata(handle);
        auto asset = GetAsset(handle);
        asset.Reset();

        m_AssetRegistry.Remove(handle);
        m_LoadedAssets.erase(handle);
        m_MemoryAssets.erase(handle);

        WriteRegistryToFile();

        fs::path filePath = Utils::File::GetAssetDirectory() / metadata.FilePath;
        Utils::File::DeleteFile(filePath.string() + ".asset");
    }

    std::unordered_set<AssetHandle> EditorAssetManager::GetAllAssetsWithType(AssetType type)
    {
        return {};
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
        metadata.LastModifiedAt = metadata.ModifiedAt;
        m_AssetRegistry[metadata.Handle] = metadata;
        CZ_CORE_TRACE("Import asset {0} , {1}.", std::to_string(metadata.Handle), metadata.IsModified());
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

    AssetHandle EditorAssetManager::LoadAsset(AssetMetadata& metadata)
    {
        // Deserialize and store asset if successful
        if (auto asset = AssetImporter::Deserialize(metadata); static_cast<bool>(asset))
        {
            metadata.IsDataLoaded = true;
            asset->Handle = metadata.Handle;
            m_LoadedAssets[metadata.Handle] = asset;
            m_AssetRegistry[metadata.Handle] = metadata;
            CZ_CORE_TRACE("Loading asset {0} from {1} finished.", std::to_string(metadata.Handle), metadata.FilePath.string());
            RegisterAssetCallback(asset);
            return metadata.Handle;
        }

        CZ_CORE_WARN("Loading asset {0} from {1} failed.", std::to_string(metadata.Handle), metadata.FilePath.string());
        metadata.IsFileMissing = true;
        return metadata.Handle;
    }

    void EditorAssetManager::SaveAssets()
    {
        for (auto& [handle, metadata] : m_AssetRegistry)
        {
            if (metadata.IsMemoryAsset)
                continue;

            if (!metadata.IsValid())
            {
                m_AssetRegistry.Remove(handle);
                continue;
            }

            if (const auto asset = GetAsset(metadata.Handle))
                SaveAsset(asset, metadata);
        }

        WriteRegistryToFile();
    }

    uint64_t EditorAssetManager::SaveAsset(Ref<Asset> asset, AssetMetadata &metadata)
    {
        uint64_t fileSize = 0;

        if (metadata.IsModified())
        {
            fileSize = AssetImporter::Serialize(metadata, asset);
            metadata.LastModifiedAt = metadata.ModifiedAt;
            CZ_CORE_TRACE("Saving Asset {0} to {1} finished.", std::to_string(metadata.Handle), metadata.FilePath.string());
        }

        return fileSize;
    }

    void EditorAssetManager::ExportAsset(Ref<Asset> asset, const fs::path &filepath)
    {
		const auto path = GetRelativePath(filepath);

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
        metadata.FileSize = SaveAsset(asset, metadata);
        metadata.LastModifiedAt = 0;

        m_LoadedAssets[asset->Handle] = asset;
        m_AssetRegistry[metadata.Handle] = metadata;

        WriteRegistryToFile();
    }

    void EditorAssetManager::RegisterAssetCallback(Ref<Asset>& asset)
    {
        asset->RegisterOnModifyCallback([this, handle = asset->Handle]()
        {
            auto metadata = m_AssetRegistry[handle];
            metadata.ModifiedAt = Utils::Time::CreateTimestamp();
            m_AssetRegistry[metadata.Handle] = metadata;
        });
    }

    AssetType EditorAssetManager::GetAssetTypeFromExtension(const std::string &extension)
    {
		const std::string ext = Utils::String::ToLowerCopy(extension);

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

    void EditorAssetManager::WriteRegistryToFile() const
    {
        AssetRegistrySerializer serializer(m_AssetRegistry);
        serializer.Serialize("../assets/AssetRegistry.czar");
    }

    AssetMetadata &EditorAssetManager::GetMetadataInternal(const AssetHandle handle)
    {
        if (m_AssetRegistry.Contains(handle))
			return m_AssetRegistry[handle];

		return s_NullMetadata;
    }
}