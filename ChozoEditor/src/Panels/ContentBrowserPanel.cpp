#include "ContentBrowserPanel.h"

#include "Chozo/ImGui/CustomTreeNode.h"
#include "Chozo/ImGui/ImGuiUI.h"
#include "Chozo/ImGui/Colors.h"

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
        m_HierachyDirectoryIcon = Texture2D::Create("../resources/icons/ContentBrowser/folder-open-1.png");
        m_DirectoryIcon = Texture2D::Create("../resources/icons/ContentBrowser/folder.png");
        m_EmptyDirectoryIcon = Texture2D::Create("../resources/icons/ContentBrowser/folder-empty.png");
        m_TextFileIcon = Texture2D::Create("../resources/icons/ContentBrowser/file.png");

        // Toolbar
        m_BackIcon = Texture2D::Create("../resources/icons/ContentBrowser/left-arrow.png");
        m_RefreshIcon = Texture2D::Create("../resources/icons/ContentBrowser/refresh.png");
        m_SearchIcon = Texture2D::Create("../resources/icons/ContentBrowser/search.png");
        m_ClearIcon = Texture2D::Create("../resources/icons/ContentBrowser/clear.png");
    }

    void ContentBrowserPanel::OnImGuiRender()
    {
        ImGui::Begin("Content Browser");
        
        ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable
            | ImGuiTableFlags_SizingFixedFit
            | ImGuiTableFlags_BordersInnerV;
        if (ImGui::BeginTable("table", 2, tableFlags, ImVec2(0.0f, 0.0f)))
        {
            ImGui::TableSetupColumn("Outliner", 0, 300.0f);
            ImGui::TableSetupColumn("Directory Structure", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);

            ImGui::BeginChild("##folders_common");
            {
                for (auto& p : std::filesystem::directory_iterator(g_AssetsPath))
                {
                    if (p.is_directory())
                        RenderDirectoryHierarchy(p);
                }
            };
            ImGui::EndChild();

            ImGui::TableSetColumnIndex(1);

            // Directory Content

            const float topBarHeight = 26.0f;
            const float bottomBarHeight = 32.0f;
            ImGui::BeginChild("##directory_structure", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetWindowHeight() - topBarHeight - bottomBarHeight));
            {
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
                RenderTopBar(topBarHeight);
                ImGui::PopStyleVar();

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
                    // Exclude files
                    if (filenameString == ".DS_Store")
                        continue;
                    
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
                // ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 64.0f, 256.0f);
                // ImGui::SliderFloat("Padding", &padding, 0.0f, 32.0f);
            };
            ImGui::EndChild();

            ImGui::EndTable();
        }

        ImGui::End();
    }

    void ContentBrowserPanel::RenderDirectoryHierarchy(std::filesystem::path directory)
    {
		std::string name = directory.filename().string();
		std::string id = name + "_TreeNode";
		// bool previousState = ImGui::TreeNodeBehaviorIsOpen(ImGui::GetID(id.c_str()));

        auto* window = ImGui::GetCurrentWindow();
        window->DC.CurrLineSize.y = 20.0f;
        window->DC.CurrLineTextBaseOffset = 3.0f;

        // const ImRect itemRect = { window->WorkRect.Min.x, window->DC.CursorPos.y,
        //                           window->WorkRect.Max.x, window->DC.CursorPos.y + window->DC.CurrLineSize.y };

        // const bool isItemClicked = [&itemRect, &id]
		// {
		// 	if (ImGui::ItemHoverable(itemRect, ImGui::GetID(id.c_str()), ImGuiItemFlags_None))
		// 	{
		// 		return ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Left);
		// 	}
		// 	return false;
		// }();

        // const bool isWindowFocused = ImGui::IsWindowFocused();

        // auto fillWithColor = [&](const ImColor& color)
		// {
		// 	const ImU32 bgColor = ImGui::ColorConvertFloat4ToU32(color);
		// 	ImGui::GetWindowDrawList()->AddRectFilled(itemRect.Min, itemRect.Max, bgColor);
		// };

		// Tree Node
        ImGuiTreeNodeFlags flags = (directory == m_CurrentDirectory ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
        bool opened = ImGui::TreeNodeWithIcon(m_HierachyDirectoryIcon, window->GetID(id.c_str()), flags, name.c_str(), NULL);

        if (ImGui::IsItemClicked())
        {
            m_CurrentDirectory = directory;
        }
        // Draw children
        if (opened)
        {
            for (auto& child : std::filesystem::directory_iterator(directory))
            {
                if (std::filesystem::is_directory(child))
                    RenderDirectoryHierarchy(child);
            }

			ImGui::TreePop();
        }
    }

    void ContentBrowserPanel::RenderTopBar(float height)
    {
        ImGui::BeginChild("##top_bar", ImVec2(0, height));
		ImGui::BeginGroup();
        {
			const float edgeOffset = 4.0f;
            // Navigation buttons
            {
                UI::ScopedStyle spacing(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 0.0f));

                auto contentBrowserButton = [height](const char* labelId, const Ref<Texture2D>& icon, bool isDisabled = false, bool isFlipped = false)
                {
					const ImU32 buttonCol = Colors::Theme::backgroundDark;
                    const ImU32 buttonColP = UI::ColourWithMultipliedValue(Colors::Theme::backgroundDark, 0.8f);
                    
                    const float iconSize = std::min(24.0f, height);
                    const float iconPadding = 2.0f;
                    ImGui::BeginDisabled(isDisabled);
                    const bool clicked = ImGui::Button(labelId, ImVec2(iconSize, iconSize));
                    ImGui::EndDisabled();
                    UI::DrawButtonImage(icon, Colors::Theme::textDarker,
                        UI::ColourWithMultipliedValue(Colors::Theme::textDarker, 1.2f),
                        UI::ColourWithMultipliedValue(Colors::Theme::textDarker, 0.8f),
                        UI::RectExpanded(UI::GetItemRect(), -iconPadding, -iconPadding),
                        isFlipped ? ImVec2(1, 0) : ImVec2(0, 0),
                        isFlipped ? ImVec2(0, 1) : ImVec2(1, 1));
                    ImGui::SameLine();
                    return clicked;
                };

                m_BackIcon_Disabled = (m_CurrentDirectory == g_AssetsPath);
                m_ForwardIcon_Disabled = (m_NextDirectory.empty());

                if (contentBrowserButton("##back", m_BackIcon, m_BackIcon_Disabled))
				{
                    OnBrowserBack();
				}

                if (contentBrowserButton("##forward", m_BackIcon, m_ForwardIcon_Disabled, true))
				{
                    OnBrowserForward();
				}

                if (contentBrowserButton("##refresh", m_RefreshIcon))
				{
                    OnBrowserRefresh();
				}
            }
        }

        // Search
        {
            UI::ShiftCursorX(10.0f);
            // UI::ShiftCursorY(2.0f);

			const float areaPosX = ImGui::GetCursorPosX();
			const float framePaddingY = ImGui::GetStyle().FramePadding.y;

			UI::ScopedStyle rounding(ImGuiStyleVar_FrameRounding, 3.0f);
			UI::ScopedStyle padding(ImGuiStyleVar_FramePadding, ImVec2(28.0f, framePaddingY));

            ImGui::SetNextItemWidth(200);

            ImGui::InputText("##Search", m_SearchBuffer, MAX_SEARCH_BUFFER_LENGTH);
            bool focused = ImGui::IsItemFocused();
			ImGui::SameLine(areaPosX + 3.0f);
            ImGui::BeginGroup();
            
            // Search icon
			const ImVec2 iconSize(ImGui::GetTextLineHeight() - 2.0f, ImGui::GetTextLineHeight() - 2.0f);
            {
                const float iconYOffset = framePaddingY;
				UI::ShiftCursorY(iconYOffset);
                ImGui::Image((ImTextureID)(uintptr_t)m_SearchIcon->GetRendererID(), iconSize, ImVec2(0, 1), ImVec2(1, 0), ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
				UI::ShiftCursorY(-iconYOffset);
                ImGui::SameLine();

                // Hint
                if (!focused)
                {
                    UI::ShiftCursorY(-framePaddingY + 1.0f);
                    UI::ScopedStyle padding(ImGuiStyleVar_FramePadding, ImVec2(0.0f, framePaddingY));
                    ImGui::TextUnformatted("Search...");
                    UI::ShiftCursorY(-1.0f);
                    ImGui::SameLine();
                }

                // Clear icon
                if (focused) {
    				const float lineHeight = ImGui::GetItemRectSize().y - framePaddingY / 2.0f;

                    UI::ShiftCursorX(150.0f);
                    UI::ShiftCursorY(framePaddingY + 1.0f);
                    if (ImGui::InvisibleButton("##ClearIcon", ImVec2{ lineHeight, lineHeight }))
                    {
                    }

                    UI::DrawButtonImage(m_ClearIcon, IM_COL32(160, 160, 160, 200),
                        IM_COL32(170, 170, 170, 255),
                        IM_COL32(160, 160, 160, 150),
                        UI::RectExpanded(UI::GetItemRect(), -2.0f, -2.0f));
                }
            }
			
            // UI::ShiftCursorY(-2.0f);

            ImGui::EndGroup();
        }

        // Breadcrumbs
        {
            UI::ShiftCursorY(-(ImGui::GetItemRectSize().y + 6.0f));
            UI::ShiftCursorX(300.0f);
            ImGui::Text("%s", m_CurrentDirectory.string().c_str());
        }
        ImGui::EndGroup();
        ImGui::SameLine();
        ImGui::EndChild();
    }

    void ContentBrowserPanel::RenderBottomBar(float height)
    {
		ImGui::BeginChild("##bottom_bar", ImVec2(0, height));
        // TODO:
		ImGui::EndChild();
    }

    void ContentBrowserPanel::OnBrowserBack()
    {
        if (!m_BackIcon_Disabled)
        {
		    m_NextDirectory = m_CurrentDirectory;
    		m_PreviousDirectory = m_CurrentDirectory.parent_path();
            m_CurrentDirectory = m_PreviousDirectory;
        }
    }

    void ContentBrowserPanel::OnBrowserForward()
    {
        m_CurrentDirectory = m_NextDirectory;
    }

    void ContentBrowserPanel::OnBrowserRefresh()
    {
    }
}
