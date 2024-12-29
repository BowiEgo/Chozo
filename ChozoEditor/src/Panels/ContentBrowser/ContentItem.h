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
    
    class ContentItem final : public RefCounted
    {
    public:
	    explicit ContentItem(Ref<DirectoryInfo> directory);
	    explicit ContentItem(const AssetMetadata& metadata);
        ~ContentItem() override = default;

        ContentBrowserItemActionResult OnImGuiRender();

        bool ShouldDelete() const { return m_Delete; }
        AssetHandle GetHandle() const { return m_Handle; }
        ImRect GetRect() const { return m_Rect; }
        std::string GetFilename() const { return m_Filename; }
        AssetType GetType() const { return m_AssetType; }
        void Delete() { m_Delete = true; }
        void Select() { m_Selected = true; }
        void Deselect() { m_Selected = false; }
    private:
        void UpdateThumbnail();
        void RenderThumbnail();
        void RenderTooltip();
        static void RenderDragDrop(const AssetHandle& handle);
        static void RenderCenteredText(const std::string& text);

        void OnDoubleClick() const;

    	void OpenMaterialPanel() const;
    private:
        ContentItemType m_Type;
        AssetType m_AssetType = AssetType::None;
        AssetHandle m_Handle;
        std::string m_Filename;
        uint64_t m_Size{};
        uint64_t m_CreatedAt{};
        uint64_t m_ModifiedAt{};
        Ref<Texture2D> m_Thumbnail;

        ImRect m_Rect;

        bool m_Delete = false;
        bool m_Selected = false;
        bool m_Empty = false;

        friend class ContentBrowserPanel;
    };

} // namespace Chozo
