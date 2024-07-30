#include "ContentBrowserPanel.h"
#include <regex>
#include <imgui_internal.h>

namespace Chozo {

    extern const std::filesystem::path g_AssetsPath = "../assets";

    extern const std::regex imagePattern(R"(\.(png|jpg|jpeg)$)", std::regex::icase);
    extern const std::regex scenePattern(R"(\.(chozo)$)", std::regex::icase);

    static ImRect GetItemRect()
	{
		return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	}

	static ImRect RectExpanded(const ImRect& rect, float x, float y)
	{
		ImRect result = rect;
		result.Min.x -= x;
		result.Min.y -= y;
		result.Max.x += x;
		result.Max.y += y;
		return result;
	}

    static void DrawButtonImage(const Ref<Texture2D>& image, ImU32 tint, ImVec2 rectMin, ImVec2 rectMax, ImVec2 uv0, ImVec2 uv1)
	{
		auto* drawList = ImGui::GetWindowDrawList();
        drawList->AddImage((ImTextureID)(uintptr_t)image->GetRendererID(), rectMin, rectMax, uv0, uv1, tint);
	};

    static void DrawButtonImage(const Ref<Texture2D>& image, ImU32 tint, ImRect rectangle, ImVec2 uv0, ImVec2 uv1)
	{
		DrawButtonImage(image, tint, rectangle.Min, rectangle.Max, uv0, uv1);
	};

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
        ImGui::InvisibleButton("##thumbnailButton", size);
        DrawButtonImage(icon, IM_COL32(255, 255, 255, 225), RectExpanded(GetItemRect(), -6.0f, -6.0f), uv0, uv1);

        ImGui::PopStyleColor();
    }

    static void DisplayCenteredText(const std::string& text)
    {
        float columnWidth = ImGui::GetContentRegionAvail().x;
        ImVec2 textSize = ImGui::CalcTextSize(text.c_str(), nullptr, false, columnWidth);
        float textX = (columnWidth - textSize.x) / 2.0f;

        if (textSize.x > columnWidth) {
            textX = 0;
        }

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textX);
        ImGui::TextWrapped("%s", text.c_str());
    }

    static void DisplayDragDrop(const std::string& relativePath)
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
        m_ImgFileIcon = Texture2D::Create("../assets/test/WechatIMG791.jpeg");
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

                    auto it = m_TextureCaches.find(filenameString);
                    if (it != m_TextureCaches.end())
                        icon = it->second;
                    else
                    {
                        Ref<Texture2D> texture = Texture2D::Create(path.string(), spec);
                        m_TextureCaches[filenameString] = texture;
                        icon = texture;
                    }
                }
                else
                    icon = m_TextFileIcon;
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
            DisplayCenteredText(filenameString);
            
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
