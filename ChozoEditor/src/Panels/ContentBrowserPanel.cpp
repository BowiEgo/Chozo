#include "ContentBrowserPanel.h"

namespace Chozo {

    static const std::filesystem::path s_AssetsPath = "../assets";

    ContentBrowserPanel::ContentBrowserPanel()
        : m_CurrentDirectory(s_AssetsPath)
    {
    }

    void ContentBrowserPanel::OnImGuiRender()
    {
        ImGui::Begin("Content Browser");

        if (m_CurrentDirectory != s_AssetsPath)
        {
            if (ImGui::Button("<-"))
            {
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
            }
        }

        for (auto& p : std::filesystem::directory_iterator(m_CurrentDirectory))
        {
            const auto& path = p.path();
            auto relativePath = std::filesystem::relative(path, s_AssetsPath);
            std::string filenameString = path.filename().string();
            if (p.is_directory())
            {
                if (ImGui::Button(filenameString.c_str()))
                {
                    m_CurrentDirectory /= path.filename();
                }
            }
            else
            {
                if (ImGui::Button(filenameString.c_str()))
                {
                }
            }
        }

        ImGui::End();
    }
}
