#pragma once

#include "Chozo.h"

namespace Chozo {

    class ContentBrowserPanel
    {
    public:
        ContentBrowserPanel();

        void OnImGuiRender();
		void RenderDirectoryHierarchy(std::filesystem::path directory);
		void RenderTopBar(float height);
    private:
        void OnBrowserBack();
        void OnBrowserForward();
        void OnBrowserRefresh();
    private:
        std::filesystem::path m_PreviousDirectory, m_CurrentDirectory, m_NextDirectory;
        Ref<Texture2D> m_DirectoryIcon, m_EmptyDirectoryIcon, m_TextFileIcon, m_BackIcon, m_RefreshIcon;
        bool m_BackIcon_Disabled, m_ForwardIcon_Disabled, m_RefreshIcon_Disabled;
        std::unordered_map<std::string, Ref<Texture2D>> m_TextureCaches;
    };
}