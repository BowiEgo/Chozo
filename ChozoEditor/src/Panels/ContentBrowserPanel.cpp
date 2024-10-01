#include "ContentBrowserPanel.h"

#include <regex>

#include "Chozo/ImGui/CustomTreeNode.h"
#include "Chozo/ImGui/ImGuiUI.h"
#include "Chozo/ImGui/Colors.h"

#include "Chozo/Utilities/PlatformUtils.h"

#include <imgui_internal.h>

namespace Chozo {

    extern const std::filesystem::path g_AssetsPath;

    extern const std::regex imagePattern;
    extern const std::regex hdrPattern;
    extern const std::regex scenePattern;

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
        UI::DrawButtonImage(icon, IM_COL32(255, 255, 255, 225), UI::RectExpanded(UI::GetItemRect(), -6.0f, -6.0f), uv0, uv1);

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

    static void DisplayDragDrop(const AssetHandle& handle)
    {
        if (ImGui::BeginDragDropSource())
        {
            std::wstring handle_wstr = std::to_wstring(handle);
            const wchar_t* handle_wchar = handle_wstr.c_str();
            ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", handle_wchar, (wcslen(handle_wchar) + 1) * sizeof(wchar_t));
            ImGui::EndDragDropSource();
        }
    }

    ContentBrowserPanel::ContentBrowserPanel()
    {
        m_ThumbnailManager = Ref<ThumbnailManager>::Create();

        m_HierachyDirectoryIcon = Texture2D::Create(std::string("../resources/icons/ContentBrowser/folder-open-1.png"));
        m_DirectoryIcon = Texture2D::Create(std::string("../resources/icons/ContentBrowser/folder.png"));
        m_EmptyDirectoryIcon = Texture2D::Create(std::string("../resources/icons/ContentBrowser/folder-empty.png"));
        m_TextFileIcon = Texture2D::Create(std::string("../resources/icons/ContentBrowser/file.png"));

        // Toolbar
        m_BackIcon = Texture2D::Create(std::string("../resources/icons/ContentBrowser/left-arrow.png"));
        m_RefreshIcon = Texture2D::Create(std::string("../resources/icons/ContentBrowser/refresh.png"));
        m_SearchIcon = Texture2D::Create(std::string("../resources/icons/ContentBrowser/search.png"));
        m_ClearIcon = Texture2D::Create(std::string("../resources/icons/ContentBrowser/clear.png"));

        OnBrowserRefresh();
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

            const float assetMenuHeight = 26.0f;
            RenderAssetMenu(assetMenuHeight);

            ImGui::BeginChild("##folders_common");
            {
                if (m_BaseDirectory)
                    RenderDirectoryHierarchy(m_BaseDirectory);
            };
            ImGui::EndChild();

            ImGui::TableSetColumnIndex(1);

            // Directory Content
            const float topBarHeight = 26.0f;
            const float bottomBarHeight = 32.0f;

            ImGui::BeginChild("##directory_structure", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetWindowHeight() - topBarHeight - bottomBarHeight));
            {
                UI::ScopedStyle frameBorderSize(ImGuiStyleVar_FrameBorderSize, 0.0f);
                RenderTopBar(topBarHeight);

                static float padding = 16.0f;
                static float thumbnailSize = 100;
                float cellSize = thumbnailSize + padding;

                float panelWidth = ImGui::GetContentRegionAvail().x;
                int columnCount = (int)(panelWidth / cellSize);
                columnCount = columnCount < 1 ? 1 : columnCount;

                ImGui::Columns(columnCount, 0, false);

                if (m_CurrentDirectory)
                {
                    for (auto& p : m_CurrentDirectory->SubDirSorted)
                    {
                        Ref<DirectoryInfo> dir = m_CurrentDirectory->SubDirectories[p];
                        auto relativePath = dir->FilePath;
                        std::string filenameString = relativePath.filename().string();
                        
                        UI::ScopedID id(filenameString.c_str());

                        Ref<Texture2D> icon;
                        icon = dir->SubDirectories.empty() ? m_EmptyDirectoryIcon : m_DirectoryIcon;
   
                        DisplayThumbnail(icon, thumbnailSize);
                        DisplayDragDrop(dir->Handle);
                        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                            m_CurrentDirectory = dir;
                        DisplayCenteredText(filenameString);
                        
                        ImGui::NextColumn();
                    }

                    for (auto& p : m_CurrentDirectory->AssetsSorted)
                    {
                        Ref<Asset> asset = Application::GetAssetManager()->GetAsset(p);
                        AssetMetadata metadata = Application::GetAssetManager()->GetMetadata(asset->Handle);
                        AssetType assetType = asset->GetAssetType();
                        std::string filenameString = metadata.FilePath.filename().string();

                        UI::ScopedID id(filenameString.c_str());

                        Ref<Texture2D> icon;

                        if (assetType == AssetType::Texture)
                            icon = m_ThumbnailManager->GetThumbnail(metadata.Handle);
                        else
                            icon = m_TextFileIcon;

                        DisplayThumbnail(icon, thumbnailSize);
                        DisplayDragDrop(metadata.Handle);
                        DisplayCenteredText(filenameString);
                        ImGui::NextColumn();
                    }
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

    void ContentBrowserPanel::RenderAssetMenu(float height)
    {
        ImGui::BeginChild("##asset_menu", ImVec2(0, height));
        UI::IconButton("Add New", IM_COL32(70, 160, 0, 255), IM_COL32(70, 180, 40, 255), IM_COL32(100, 190, 40, 255));
        ImGui::SameLine();
        
        if (UI::IconButton("Import", IM_COL32(210, 130, 0, 255), IM_COL32(210, 150, 40, 255), IM_COL32(240, 160, 40, 255)))
        {
            ImportAssets();
        }
        ImGui::SameLine();
        
        if (UI::IconButton("Save All", IM_COL32(33, 33, 33, 255), IM_COL32(55, 55, 55, 255), IM_COL32(77, 77, 77, 255)))
        {
            SaveAllAssets();
        }
        ImGui::SameLine();

        ImGui::EndChild();
    }

    void ContentBrowserPanel::RenderDirectoryHierarchy(Ref<DirectoryInfo> directory)
    {
		std::string name = directory == m_BaseDirectory ? "All" : directory->FilePath.filename().string();
		std::string id = name + "_TreeNode";
		// bool previousState = ImGui::TreeNodeBehaviorIsOpen(ImGui::GetID(id.c_str()));

        auto* window = ImGui::GetCurrentWindow();
        // window->DC.CurrLineSize.y = 20.0f;
        // window->DC.CurrLineTextBaseOffset = 3.0f;

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
		UI::ScopedStyle padding(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));

        ImGuiTreeNodeFlags flags = (directory == m_CurrentDirectory ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
        if (directory == m_BaseDirectory)
            flags |= ImGuiTreeNodeFlags_DefaultOpen;
        if (directory->SubDirectories.empty())
            flags |= ImGuiTreeNodeFlags_Leaf;

        bool opened = ImGui::TreeNodeWithIcon(m_HierachyDirectoryIcon, window->GetID(id.c_str()), flags, name.c_str(), NULL);

        if (ImGui::IsItemClicked())
        {
            m_CurrentDirectory = directory;
        }
        // Draw children
        if (opened)
        {
            for (auto handle : directory->SubDirSorted)
                RenderDirectoryHierarchy(directory->SubDirectories[handle]);

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

                m_BackIcon_Disabled = (m_CurrentDirectory == m_BaseDirectory);
                m_ForwardIcon_Disabled = (m_NextDirectory == nullptr);

                if (contentBrowserButton("##back", m_BackIcon, m_BackIcon_Disabled))
                    OnBrowserBack();

                if (contentBrowserButton("##forward", m_BackIcon, m_ForwardIcon_Disabled, true))
                    OnBrowserForward();

                if (contentBrowserButton("##refresh", m_RefreshIcon))
                    OnBrowserRefresh();
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
            if (m_CurrentDirectory)
                ImGui::Text("%s", m_CurrentDirectory->FilePath.string().c_str());
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

    void ContentBrowserPanel::ImportAssets()
    {
        std::filesystem::path path = FileDialogs::OpenFile("Import (*.png)\0*.jpeg\0");

        Texture2DSpecification spec;
        spec.WrapS = ImageParameter::CLAMP_TO_BORDER;
        spec.WrapT = ImageParameter::CLAMP_TO_BORDER;

        Ref<Asset> textureAsset = CreateAsset<Texture2D>(path.filename().stem().string(), m_CurrentDirectory, path.string(), spec);

        SaveAllAssets();
    }

    void ContentBrowserPanel::SaveAllAssets()
    {
        Application::GetAssetManager()->SaveAssets();
    }

    void ContentBrowserPanel::AddAssetsToDir(Ref<DirectoryInfo> directory, AssetMetadata& metadata)
    {
        directory->Assets[metadata.Handle] = Application::GetAssetManager()->GetMetadata(metadata.Handle);
        directory->AssetsSorted.push_back(metadata.Handle);
    }

    void ContentBrowserPanel::SortAssets(Ref<DirectoryInfo> directory)
    {
        std::sort(directory->AssetsSorted.begin(), directory->AssetsSorted.end(), 
            [&](const auto& a, const auto& b) {
                const auto& assetA = directory->Assets[a].FilePath.filename().string();
                const auto& assetB = directory->Assets[b].FilePath.filename().string();
                return assetA < assetB;
        });
    }

    void ContentBrowserPanel::SortSubDirs(Ref<DirectoryInfo> directory)
    {
        std::sort(directory->SubDirSorted.begin(), directory->SubDirSorted.end(), 
            [&](const auto& a, const auto& b) {
                const auto& dirA = directory->SubDirectories[a]->FilePath.filename().string();
                const auto& dirB = directory->SubDirectories[b]->FilePath.filename().string();
                return dirA < dirB;
        });
    }

    void ContentBrowserPanel::OnBrowserBack()
    {
        if (!m_BackIcon_Disabled)
        {
		    m_NextDirectory = m_CurrentDirectory;
    		m_PreviousDirectory = m_CurrentDirectory->Parent;
            m_CurrentDirectory = m_PreviousDirectory;
        }
    }

    void ContentBrowserPanel::OnBrowserForward()
    {
        m_CurrentDirectory = m_NextDirectory;
    }

    void ContentBrowserPanel::OnBrowserRefresh()
    {
		AssetHandle baseDirectoryHandle = ProcessDirectory(g_AssetsPath, nullptr);
		m_BaseDirectory = m_Directories[baseDirectoryHandle];
    }

    AssetHandle ContentBrowserPanel::ProcessDirectory(const std::filesystem::path &directoryPath, const Ref<DirectoryInfo> &parent)
    {
		Ref<DirectoryInfo> directoryInfo = Ref<DirectoryInfo>::Create();
        directoryInfo->Handle = AssetHandle();
        directoryInfo->Parent = parent;

		if (directoryPath == g_AssetsPath)
            directoryInfo->FilePath = "";
        else
			directoryInfo->FilePath = std::filesystem::relative(directoryPath, g_AssetsPath);

		for (auto entry : std::filesystem::directory_iterator(directoryPath))
        {
            if (entry.is_directory())
			{
				AssetHandle subdirHandle = ProcessDirectory(entry.path(), directoryInfo);
				directoryInfo->SubDirectories[subdirHandle] = m_Directories[subdirHandle];
                directoryInfo->SubDirSorted.push_back(subdirHandle);
				continue;
			}

			auto metadata = Application::GetAssetManager()->GetMetadata(entry.path());
            if (metadata.IsValid())
                AddAssetsToDir(directoryInfo, metadata);
        }

        SortSubDirs(directoryInfo);
        SortAssets(directoryInfo);

		m_Directories[directoryInfo->Handle] = directoryInfo;

        return directoryInfo->Handle;
    }
}
