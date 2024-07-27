#include "ContentBrowserPanel.h"
#include <regex>

namespace Chozo {

    extern const std::filesystem::path g_AssetsPath = "../assets";

    std::regex imagePattern(R"(\.(png|jpg|jpeg)$)", std::regex::icase);

    static void DisplayThumbnail(const Ref<Texture2D>& icon, float thumbnailSize)
    {
        float imageAspectRatio = static_cast<float>(icon->GetHeight()) / static_cast<float>(icon->GetWidth());
        ImVec2 uv0(0.0f, 1.0f);
        ImVec2 uv1(1.0f, 0.0f);
        ImVec2 size(thumbnailSize, thumbnailSize);

        if (imageAspectRatio <= 1.0f) {
            float offsetY = (1.0f - 1.0f / imageAspectRatio) / 2.0f;
            uv0.y = 1.0f - offsetY;
            uv1.y = offsetY;
        } else {
            float offsetX = (1.0f - imageAspectRatio) / 2.0f;
            uv0.x = offsetX;
            uv1.x = 1.0f - offsetX;
        }

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::ImageButton((void*)(uintptr_t)icon->GetRendererID(), size, uv0, uv1);

        ImGui::PopStyleColor();
    }

    void DisplayDragDrop(const std::string& relativePath)
    {
        if (ImGui::BeginDragDropSource())
        {
            const wchar_t* itemPath = (const wchar_t*)relativePath.c_str();
            ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
            ImGui::EndDragDropSource();
        }    
    }

    ContentBrowserPanel::ContentBrowserPanel()
        : m_CurrentDirectory(g_AssetsPath)
    {
        m_DirectoryIcon = Texture2D::Create("../resources/icons/ContentBrowser/folder.png");
        m_EmptyDirectoryIcon = Texture2D::Create("../resources/icons/ContentBrowser/folder-empty.png");
        m_TextFileIcon = Texture2D::Create("../resources/icons/ContentBrowser/file.png");
    }

    void ContentBrowserPanel::OnImGuiRender()
    {
        ImGui::Begin("Content Browser");

        if (m_CurrentDirectory != g_AssetsPath)
        {
            if (ImGui::Button("<-"))
            {
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
            }
        }

        static float padding = 16.0f;
        static float thumbnailSize = 100;
        float cellSize = thumbnailSize + padding;

        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int)(panelWidth / cellSize);
        columnCount = columnCount < 1 ? 1 : columnCount;

        ImGui::Columns(columnCount, 0, false);

        for (auto& p : std::filesystem::directory_iterator(m_CurrentDirectory))
        {
            const auto& path = p.path();
            auto relativePath = std::filesystem::relative(path, g_AssetsPath);
            std::string filenameString = path.filename().string();
            
            ImGui::PushID(filenameString.c_str());

            Ref<Texture2D> icon;
            Texture2DSpecification spec;
            spec.wrapS = Texture2DParameter::CLAMP_TO_BORDER;
            spec.wrapT = Texture2DParameter::CLAMP_TO_BORDER;

            if (p.is_directory())
            {
                icon = std::filesystem::is_empty(path) ? m_EmptyDirectoryIcon : m_DirectoryIcon;
            }
            else
            {
                std::string fileExtension = path.extension().string();
                if (std::regex_match(fileExtension, imagePattern))
                {
                    auto it = m_TextureCaches.find(path.string());
                    if (it != m_TextureCaches.end())
                    {
                        icon = it->second;
                    }
                    else
                    {
                        Ref<Texture2D> texture = Texture2D::Create(path.string(), spec);
                        m_TextureCaches[filenameString] = texture;
                        icon = texture;
                    }
                }
                else
                {
                    icon = m_TextFileIcon;
                }
            }

            DisplayThumbnail(icon, thumbnailSize);
            DisplayDragDrop(relativePath);
            
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                if (p.is_directory())
                {
                    m_CurrentDirectory /= path.filename();
                }
            }
            ImGui::TextWrapped("%s", filenameString.c_str());
            
            ImGui::NextColumn();
            ImGui::PopID();
        }

        ImGui::Columns(1);

        // Status bar
        ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 64.0f, 256.0f);
        ImGui::SliderFloat("Padding", &padding, 0.0f, 32.0f);

        ImGui::End();
    }
}
