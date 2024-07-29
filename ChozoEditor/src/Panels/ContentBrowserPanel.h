#pragma once

#include "Chozo.h"

namespace Chozo {

    class ContentBrowserPanel
    {
    public:
        ContentBrowserPanel();

        void OnImGuiRender();
    private:
        std::filesystem::path m_CurrentDirectory;
        Ref<Texture2D> m_DirectoryIcon, m_EmptyDirectoryIcon, m_TextFileIcon, m_ImgFileIcon;
        std::unordered_map<std::string, Ref<Texture2D>> m_TextureCaches;
    };
}