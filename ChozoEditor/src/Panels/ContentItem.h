#pragma once

#include "Chozo/Asset/Asset.h"
#include "Chozo/Renderer/Texture.h"

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

        inline bool ShouldDelete() { return m_Delete; }
        inline AssetHandle GetHandle() { return m_Handle; }

        void OnImGuiRender();
    private:
        void RenderThumbnail(const Ref<Texture2D>& icon, float thumbnailSize);
        void RenderTooltip();
        void RenderContexMenu();
        void RenderDragDrop(const AssetHandle& handle);
        void RenderCenteredText(const std::string& text);

        void OnDoubleClick();
    private:
        ContentItemType m_Type;
        AssetHandle m_Handle;
        std::string m_Filename;
        uint64_t m_Size;
        Ref<Texture2D> m_Thumbnail;
        bool m_Delete = false;
    };

} // namespace Chozo
