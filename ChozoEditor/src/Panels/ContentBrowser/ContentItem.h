#pragma once

#include "Chozo/Asset/Asset.h"
#include "Chozo/Renderer/Texture.h"

#include "Chozo/ImGui/ImGuiUI.h"

namespace Chozo {

    class ContentBrowserPanel;

    struct DirectoryInfo : public RefCounted
	{
		AssetHandle Handle;
		Ref<DirectoryInfo> Parent;

		fs::path FilePath = "";

		std::map<AssetHandle, AssetMetadata> Assets;
		std::vector<AssetHandle> AssetsSorted;

		std::map<AssetHandle, Ref<DirectoryInfo>> SubDirectories;
        std::vector<AssetHandle> SubDirSorted;
	};

    enum class ContentItemType {
        Directory,
        Asset,
    };
    
    class ContentItem : public RefCounted
    {
    public:
        ContentItem(Ref<DirectoryInfo> directory);
        ContentItem(AssetMetadata metadata);
        ~ContentItem() = default;

        void OnImGuiRender();

        inline bool ShouldDelete() { return m_Delete; }
        inline bool IsHovered() { return m_Hovered; }
        inline AssetHandle GetHandle() const { return m_Handle; }
        inline std::pair<ImVec2, ImVec2> GetRect() const { return m_Rect; }
        inline void Delete() { m_Delete = true; }
        inline void Select() { m_Select = true; }
        inline void Deselect() { m_Select = false; }
    private:
        void RenderThumbnail(const Ref<Texture2D>& icon, float thumbnailSize);
        void RenderTooltip();
        void RenderDragDrop(const AssetHandle& handle);
        void RenderCenteredText(const std::string& text);

        void OnDoubleClick();
    private:
        ContentItemType m_Type;
        AssetType m_AssetType;
        AssetHandle m_Handle;
        std::string m_Filename;
        uint64_t m_Size;
        Ref<Texture2D> m_Thumbnail;

        std::pair<ImVec2, ImVec2> m_Rect;

        bool m_Delete = false;
        bool m_Select = false;
        bool m_Hovered = false;
    };

} // namespace Chozo
