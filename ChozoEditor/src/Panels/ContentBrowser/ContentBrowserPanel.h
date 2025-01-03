#pragma once

#include "Chozo/Core/Application.h"
#include "Chozo/Scene/Scene.h"
#include "Chozo/Asset/EditorAssetManager.h"

#include "ContentItem.h"
#include "ContentSelection.h"

namespace Chozo {

    #define MAX_SEARCH_BUFFER_LENGTH 24

    class ContentBrowserPanel
    {
    public:
        ContentBrowserPanel();

        void OnImGuiRender();

        void ChangeDirectory(Ref<DirectoryInfo>& directory);
        void ChangeDirectory(AssetHandle directoryHandle);
        void OnDirectoryChange();
        void OnBrowserBack();
        void OnBrowserForward();
        void OnBrowserRefresh();
    public:
		inline static ContentBrowserPanel& Get() { return *s_Instance; }
        inline static Ref<Texture2D> GetIcon(const std::string& name) { return s_Instance->m_Icons[name]; }
        inline static std::vector<Ref<ContentItem>> GetItems() { return s_Instance->m_CurrentItems; }
        inline static Ref<ContentItem> GetHoveredItem() { return s_Instance->m_HoveredItem; }

        inline static void Select(Ref<ContentItem> item) { item->Select(); }
        inline static void Select(AssetHandle handle) {
            if (s_Instance->m_CurrentItems[handle])
                s_Instance->m_CurrentItems[handle]->Select();
        }
        inline static void Deselect(Ref<ContentItem> item) { item->Deselect(); }
        inline static void Deselect(AssetHandle handle) {
            if (s_Instance->m_CurrentItems[handle])
                s_Instance->m_CurrentItems[handle]->Deselect();
        }
        static Ref<Scene> CreateScene();
        static Ref<Material> CreateMaterial();
    private:
        static void RenderAddNewContextMenu();
        void RenderItems();
        void RenderItemContextMenu();
		void RenderAssetMenu(float height);
		void RenderDirectoryHierarchy(Ref<DirectoryInfo> directory);
		void RenderTopBar(float height);
		static void RenderBottomBar(float height);

        void ImportAssets();
		template<typename T, typename... Args>
        Ref<T> CreateAsset(const std::string& filename, Ref<DirectoryInfo>& directory, Args&&... args);
        void SaveAllAssets();
        static void RenderAssetThumbnails(const std::vector<AssetMetadata>& metadatas);

        static void AddAssetToDir(Ref<DirectoryInfo> directory, const AssetMetadata& metadata);
        static void RemoveAssetFromDir(Ref<DirectoryInfo> directory, AssetHandle handle);
        static void SortAssets(Ref<DirectoryInfo> directory);
        static void SortSubDirs(Ref<DirectoryInfo> directory);

		AssetHandle ProcessDirectory(const fs::path& directoryPath, const Ref<DirectoryInfo>& parent);
        Ref<ContentItem> GetItem(AssetHandle handle);
        void DeleteItem(Ref<ContentItem> item);
        void DeleteItems(const std::vector<Ref<ContentItem>>& items);

        static std::string CreateItemName(AssetType type);
    public:
        static float s_Padding;
        static float s_ThumbnailSize;
    private:
		static ContentBrowserPanel* s_Instance;

        std::unordered_map<std::string, Ref<Texture2D>> m_Icons;
		std::unordered_map<AssetHandle, Ref<DirectoryInfo>> m_Directories;
		Ref<DirectoryInfo> m_BaseDirectory;
        Ref<DirectoryInfo> m_PreviousDirectory, m_CurrentDirectory, m_NextDirectory;

        std::vector<Ref<ContentItem>> m_CurrentItems;
        std::vector<Ref<ContentItem>> m_ItemsShouldDelete;
        std::vector<Ref<ContentItem>> m_ItemsSelected;
        Ref<ContentItem> m_HoveredItem;

        ContentSelection m_ContentSelection;

        bool m_BackIcon_Disabled{}, m_ForwardIcon_Disabled{}, m_RefreshIcon_Disabled{};

		char m_SearchBuffer[MAX_SEARCH_BUFFER_LENGTH]{};
    };
    
    template <typename T, typename... Args>
    inline Ref<T> ContentBrowserPanel::CreateAsset(const std::string& filename, Ref<DirectoryInfo>& directory, Args &&...args)
    {
        fs::path filePath = Utils::File::GetAssetDirectory() / directory->FilePath;
        Ref<T> asset = Application::GetAssetManager()->CreateNewAsset<T>(filename, filePath.string(), std::forward<Args>(args)...);
        auto metadata = Application::GetAssetManager()->GetMetadata(asset->Handle);

        AddAssetToDir(directory, metadata);
        SortAssets(directory);

        return asset;
    }
}