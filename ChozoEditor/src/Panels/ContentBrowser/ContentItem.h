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

    enum class ContentBrowserAction
	{
		None                = 0,
		Selected			= BIT(0),
		Deselected			= BIT(1),
		Hovered             = BIT(2),
		Deleted             = BIT(3),
		Activated			= BIT(4)
	};

    struct ContentBrowserItemActionResult
	{
		uint16_t Field = 0;

		void Set(ContentBrowserAction flag, bool value)
		{
			if (value)
				Field |= (uint16_t)flag;
			else
				Field &= ~(uint16_t)flag;
		}

		bool IsSet(ContentBrowserAction flag) const { return (uint16_t)flag & Field; }
	};

    // struct ImRect
    // {
    //     ImVec2 Min, Max;

    //     bool operator==(const ImRect& other) const {
    //         return Min.x == other.Min.x && Min.y == other.Min.y && Max.x == other.Max.x && Max.y == other.Max.y;
    //     }
    // };
    
    class ContentItem : public RefCounted
    {
    public:
        ContentItem(Ref<DirectoryInfo> directory);
        ContentItem(const AssetMetadata& metadata);
        ~ContentItem() = default;

        ContentBrowserItemActionResult OnImGuiRender();

        inline bool ShouldDelete() { return m_Delete; }
        inline AssetHandle GetHandle() const { return m_Handle; }
        inline ImRect GetRect() const { return m_Rect; }
        inline std::string GetFilename() const { return m_Filename; }
        inline void Delete() { m_Delete = true; }
        inline void Select() { m_Selected = true; }
        inline void Deselect() { m_Selected = false; }
    private:
        void UpdateThumbnail();
        void RenderThumbnail();
        void RenderTooltip();
        static void RenderDragDrop(const AssetHandle& handle);
        static void RenderCenteredText(const std::string& text);

        void OnDoubleClick() const;
    private:
        ContentItemType m_Type;
        AssetType m_AssetType = AssetType::None;
        AssetHandle m_Handle;
        std::string m_Filename;
        uint64_t m_Size{};
        uint64_t m_CreateAt{};
        Ref<Texture2D> m_Thumbnail;

        ImRect m_Rect;

        bool m_Delete = false;
        bool m_Selected = false;
        bool m_Empty = false;

        friend class ContentBrowserPanel;
    };

} // namespace Chozo
