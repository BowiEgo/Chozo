#pragma once

#include "Chozo.h"
#include "Chozo/Asset/EditorAssetManager.h"

namespace Chozo {

    #define MAX_SEARCH_BUFFER_LENGTH 24

    struct DirectoryInfo : public RefCounted
	{
		AssetHandle Handle;
		Ref<DirectoryInfo> Parent;

		std::filesystem::path FilePath;

		std::vector<AssetHandle> Assets;

		std::map<AssetHandle, Ref<DirectoryInfo>> SubDirectories;
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
        Ref<T> CreateAsset(const std::string& filepath, Args&&... args);

        void OnBrowserBack();
        void OnBrowserForward();
        void OnBrowserRefresh();

		AssetHandle ProcessDirectory(const std::filesystem::path& directoryPath, const Ref<DirectoryInfo>& parent);
    private:
		Ref<EditorAssetManager> m_AssetManager;

		std::unordered_map<AssetHandle, Ref<DirectoryInfo>> m_Directories;
		Ref<DirectoryInfo> m_BaseDirectory;
        Ref<DirectoryInfo> m_PreviousDirectory, m_CurrentDirectory, m_NextDirectory;

        Ref<Texture2D> m_HierachyDirectoryIcon, m_DirectoryIcon, m_EmptyDirectoryIcon, m_TextFileIcon,
            m_BackIcon, m_RefreshIcon, m_SearchIcon, m_ClearIcon;
        bool m_BackIcon_Disabled, m_ForwardIcon_Disabled, m_RefreshIcon_Disabled;
        std::unordered_map<std::string, Ref<Texture2D>> m_TextureCaches;

		char m_SearchBuffer[MAX_SEARCH_BUFFER_LENGTH];
    };
    
    template <typename T, typename... Args>
    inline Ref<T> ContentBrowserPanel::CreateAsset(const std::string &path, Args &&...args)
    {
        std::filesystem::path filePath(path);
        Ref<T> asset = m_AssetManager->CreateNewAsset<T>(filePath.filename().string(), m_CurrentDirectory->FilePath.string(), std::forward<Args>(args)...);

        return asset;
    }
}