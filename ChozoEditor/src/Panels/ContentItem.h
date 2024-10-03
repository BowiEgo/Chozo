#pragma once

#include "Chozo/Asset/Asset.h"
#include "Chozo/Renderer/Texture.h"

namespace Chozo {

    class ContentBrowserPanel;

    struct DirectoryInfo : public RefCounted
	{
		AssetHandle Handle;
		Ref<DirectoryInfo> Parent;

		std::filesystem::path FilePath = "";

		std::map<AssetHandle, AssetMetadata> Assets;
		std::vector<AssetHandle> AssetsSorted;

		std::map<AssetHandle, Ref<DirectoryInfo>> SubDirectories;
        std::vector<AssetHandle> SubDirSorted;
	};

    enum class ContentItemType {
        Directory,
        Asset,
    };
    
    class ContentItem
    {
    public:
        ContentItem(Ref<DirectoryInfo> directory);
        ContentItem(AssetMetadata metadata);
        ~ContentItem() = default;

        void OnImGuiRender();
        void RenderTooltip();
    private:
        void OnDoubleClick();
    private:
        ContentItemType m_Type;
        AssetHandle m_Handle;
        std::string m_Filename;
        uint64_t m_Size;
        Ref<Texture2D> m_Thumbnail;
    };

} // namespace Chozo
