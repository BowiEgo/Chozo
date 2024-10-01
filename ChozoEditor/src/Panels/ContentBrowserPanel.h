#pragma once

#include "Chozo.h"
#include "Chozo/Asset/EditorAssetManager.h"
#include "Chozo/Thumbnail/ThumbnailManager.h"

namespace Chozo {

    #define MAX_SEARCH_BUFFER_LENGTH 24

    struct DirectoryInfo : public RefCounted
	{
		AssetHandle Handle;
		Ref<DirectoryInfo> Parent;

		std::filesystem::path FilePath;

		std::map<AssetHandle, AssetMetadata> Assets;
		std::vector<AssetHandle> AssetsSorted;

		std::map<AssetHandle, Ref<DirectoryInfo>> SubDirectories;
        std::vector<AssetHandle> SubDirSorted;
	};

    class ContentBrowserPanel
    {
    public:
        ContentBrowserPanel();

        void OnImGuiRender();
		void RenderAssetMenu(float height);
		void RenderDirectoryHierarchy(Ref<DirectoryInfo> directory);
		void RenderTopBar(float height);
		void RenderBottomBar(float height);
    private:
        void ImportAssets();
		template<typename T, typename... Args>
        Ref<T> CreateAsset(const std::string& filename, Ref<DirectoryInfo>& directory, Args&&... args);
        void SaveAllAssets();

        void AddAssetsToDir(Ref<DirectoryInfo> directory, AssetMetadata& metadata);
        void SortAssets(Ref<DirectoryInfo> directory);
        void SortSubDirs(Ref<DirectoryInfo> directory);

        void OnBrowserBack();
        void OnBrowserForward();
        void OnBrowserRefresh();

		AssetHandle ProcessDirectory(const std::filesystem::path& directoryPath, const Ref<DirectoryInfo>& parent);
    private:
		std::unordered_map<AssetHandle, Ref<DirectoryInfo>> m_Directories;
		Ref<DirectoryInfo> m_BaseDirectory;
        Ref<DirectoryInfo> m_PreviousDirectory, m_CurrentDirectory, m_NextDirectory;

        Ref<Texture2D> m_HierachyDirectoryIcon, m_DirectoryIcon, m_EmptyDirectoryIcon, m_TextFileIcon,
            m_BackIcon, m_RefreshIcon, m_SearchIcon, m_ClearIcon;
        bool m_BackIcon_Disabled, m_ForwardIcon_Disabled, m_RefreshIcon_Disabled;

		char m_SearchBuffer[MAX_SEARCH_BUFFER_LENGTH];

        Ref<ThumbnailManager> m_ThumbnailManager;
    };
    
    template <typename T, typename... Args>
    inline Ref<T> ContentBrowserPanel::CreateAsset(const std::string& filename, Ref<DirectoryInfo>& directory, Args &&...args)
    {
        Ref<T> asset = Application::GetAssetManager()->CreateNewAsset<T>(filename, directory->FilePath.string(), std::forward<Args>(args)...);
        auto metadata = Application::GetAssetManager()->GetMetadata(asset->Handle);

        AddAssetsToDir(directory, metadata);
        SortAssets(directory);

        m_ThumbnailManager->CreateThumbnail(metadata, asset);

        return asset;
    }
}