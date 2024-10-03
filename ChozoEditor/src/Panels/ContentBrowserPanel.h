#pragma once

#include "Chozo/Core/Application.h"
#include "Chozo/Asset/EditorAssetManager.h"
#include "Chozo/Thumbnail/ThumbnailManager.h"

#include "ContentItem.h"

namespace Chozo {

    #define MAX_SEARCH_BUFFER_LENGTH 24

    class ContentBrowserPanel
    {
    public:
        ContentBrowserPanel();

        void OnImGuiRender();

        void ChangeDirectory(Ref<DirectoryInfo> directory);
        void ChangeDirectory(AssetHandle directoryHandle);
        void OnDirectoryChange(Ref<DirectoryInfo> directory);
        void OnBrowserBack();
        void OnBrowserForward();
        void OnBrowserRefresh();
        inline Ref<ThumbnailManager> GetThumbnailManager() { return m_ThumbnailManager; }
    public:
        inline static Ref<Texture2D> GetIcon(std::string name) { return s_Icons[name]; }
		inline static ContentBrowserPanel& Get() { return *s_Instance; }
    private:
		void RenderAssetMenu(float height);
		void RenderDirectoryHierarchy(Ref<DirectoryInfo> directory);
		void RenderTopBar(float height);
		void RenderBottomBar(float height);

        void ImportAssets();
		template<typename T, typename... Args>
        Ref<T> CreateAsset(const std::string& filename, Ref<DirectoryInfo>& directory, Args&&... args);
        void SaveAllAssets();

        void AddAssetsToDir(Ref<DirectoryInfo> directory, AssetMetadata& metadata);
        void SortAssets(Ref<DirectoryInfo> directory);
        void SortSubDirs(Ref<DirectoryInfo> directory);

		AssetHandle ProcessDirectory(const fs::path& directoryPath, const Ref<DirectoryInfo>& parent);
        void DeleteItem(Ref<ContentItem> item);
    public:
        static float s_Padding;
        static float s_ThumbnailSize;
    private:
		static ContentBrowserPanel* s_Instance;
        static std::unordered_map<std::string, Ref<Texture2D>> s_Icons;

		std::unordered_map<AssetHandle, Ref<DirectoryInfo>> m_Directories;
		Ref<DirectoryInfo> m_BaseDirectory;
        Ref<DirectoryInfo> m_PreviousDirectory, m_CurrentDirectory, m_NextDirectory;

        std::vector<Ref<ContentItem>> m_CurrentItems;

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