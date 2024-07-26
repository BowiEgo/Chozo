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
    };
}