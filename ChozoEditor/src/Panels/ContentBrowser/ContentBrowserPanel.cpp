#include "ContentBrowserPanel.h"

#include <regex>

#include "Chozo/ImGui/CustomTreeNode.h"
#include "Chozo/ImGui/ImGuiUI.h"
#include "Chozo/ImGui/Colors.h"

#include "Chozo/Utilities/PlatformUtils.h"
#include "Chozo/Renderer/Material.h"
#include "Chozo/Core/Pool.h"

#include "Thumbnail/ThumbnailManager.h"
#include <imgui_internal.h>

namespace Chozo {

	float ContentBrowserPanel::s_Padding = 16.0f;
	float ContentBrowserPanel::s_ThumbnailSize = 100.0f;
	ContentBrowserPanel* ContentBrowserPanel::s_Instance;
    std::unordered_map<std::string, Ref<Texture2D>> ContentBrowserPanel::s_Icons;

    ContentBrowserPanel::ContentBrowserPanel()
    {
		s_Instance = this;

        s_Icons["HierachyDirectory"] = Texture2D::Create(std::string("../resources/icons/ContentBrowser/folder-open-1.png"));
        s_Icons["Directory"] = Texture2D::Create(std::string("../resources/icons/ContentBrowser/folder.png"));
        s_Icons["EmptyDirectory"] = Texture2D::Create(std::string("../resources/icons/ContentBrowser/folder-empty.png"));
        s_Icons["TextFile"] = Texture2D::Create(std::string("../resources/icons/ContentBrowser/file.png"));
        // Toolbar
        s_Icons["Back"] = Texture2D::Create(std::string("../resources/icons/ContentBrowser/left-arrow.png"));
        s_Icons["Refresh"] = Texture2D::Create(std::string("../resources/icons/ContentBrowser/refresh.png"));
        s_Icons["Search"] = Texture2D::Create(std::string("../resources/icons/ContentBrowser/search.png"));
        s_Icons["Clear"] = Texture2D::Create(std::string("../resources/icons/ContentBrowser/clear.png"));

        m_ContentSelection = ContentSelection();

        OnBrowserRefresh();
    }

    void ContentBrowserPanel::OnImGuiRender()
    {
        std::vector<Ref<ContentItem>> itemsDeleted;
        for (auto item : m_CurrentItems)
        {
            if (item && item->ShouldDelete())
                itemsDeleted.push_back(item);
        }
        DeleteItems(itemsDeleted);

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

                float cellSize = s_ThumbnailSize + s_Padding;

                float panelWidth = ImGui::GetContentRegionAvail().x;
                int columnCount = (int)(panelWidth / cellSize);
                columnCount = columnCount < 1 ? 1 : columnCount;

                ImGui::Columns(columnCount, 0, false);

                if (m_CurrentDirectory)
                {
                    m_HoveredItem = nullptr;
                    for (auto item : m_CurrentItems)
                    {
                        if (!item->ShouldDelete())
                        {
                            item->OnImGuiRender();

                            if (item->IsHovered())
                                m_HoveredItem = item;
                                
                            ImGui::NextColumn();
                        }
                    }
                }

                ImGui::Columns(1);

                m_ContentSelection.OnImGuiRender();

                RenderItemContextMenu();
                if (m_HoveredItem)
                {
                    if (ImGui::IsMouseClicked(0))
                    {
                        ImGui::CloseCurrentPopup();
                        m_ContentSelection.Select(m_HoveredItem);
                    }

                    if (ImGui::IsMouseClicked(1))
                    {
                        if (m_ContentSelection.Size() <= 1)
                        {
                            m_ContentSelection.Select(m_HoveredItem);
                        }
                        ImGui::OpenPopup("ItemContextMenu");
                    }
                }
                else
                {
                    if (ImGui::IsMouseClicked(0))
                    {
                        if (!ImGui::IsPopupOpen("ItemContextMenu"))
                            m_ContentSelection.Clear();

                        ImGui::CloseCurrentPopup();
                    }
                }

                // Status bar
                // ImGui::SliderFloat("Thumbnail Size", &s_ThumbnailSize, 64.0f, 256.0f);
                // ImGui::SliderFloat("Padding", &s_Padding, 0.0f, 32.0f);
            };

            ImGui::EndChild();
            ImGui::EndTable();
        }

        ImGui::End();
    }

    void ContentBrowserPanel::ChangeDirectory(Ref<DirectoryInfo> directory)
    {
        m_NextDirectory = nullptr;
        m_PreviousDirectory = m_CurrentDirectory;
        m_CurrentDirectory = directory;

        OnDirectoryChange(directory);
    }

    void ContentBrowserPanel::ChangeDirectory(AssetHandle directoryHandle)
    {
        ChangeDirectory(m_Directories[directoryHandle]);
    }

    void ContentBrowserPanel::OnDirectoryChange(Ref<DirectoryInfo> directory)
    {
        if (!m_CurrentDirectory)
            return;

        m_CurrentItems.clear();

        for (auto& [subdirHandle, subDir] : directory->SubDirectories)
            m_CurrentItems.push_back(Ref<ContentItem>::Create(subDir));
        
        for (auto& [AssetHandle, metadata] : directory->Assets)
            m_CurrentItems.push_back(Ref<ContentItem>::Create(metadata));
    }

    void ContentBrowserPanel::OnBrowserBack()
    {
        if (!m_BackIcon_Disabled)
        {
		    m_NextDirectory = m_CurrentDirectory;
    		m_PreviousDirectory = m_CurrentDirectory->Parent;
            m_CurrentDirectory = m_PreviousDirectory;
            OnDirectoryChange(m_CurrentDirectory);
        }
    }

    void ContentBrowserPanel::OnBrowserForward()
    {
        m_CurrentDirectory = m_NextDirectory;
        OnDirectoryChange(m_CurrentDirectory);
    }

    void ContentBrowserPanel::OnBrowserRefresh()
    {
		AssetHandle baseDirectoryHandle = ProcessDirectory(Utils::File::GetAssetDirectory(), nullptr);
		m_BaseDirectory = m_Directories[baseDirectoryHandle];
        OnDirectoryChange(m_CurrentDirectory);
    }

    Ref<Scene> ContentBrowserPanel::CreateScene()
    {
        if (!s_Instance->m_CurrentDirectory)
        {
            CZ_CORE_ERROR("ContentBrowser: Current directory is not exist!");
            return nullptr;
        }

        auto filename = s_Instance->CreateItemName(AssetType::Scene);

        auto scene = s_Instance->CreateAsset<Scene>(filename, s_Instance->m_CurrentDirectory);
        s_Instance->OnBrowserRefresh();
        return scene;
    }

    Ref<Material> ContentBrowserPanel::CreateMaterial()
    {
        if (!s_Instance->m_CurrentDirectory)
        {
            CZ_CORE_ERROR("ContentBrowser: Current directory is not exist!");
            return nullptr;
        }

        auto filename = s_Instance->CreateItemName(AssetType::Material);

        auto material = s_Instance->CreateAsset<Material>(filename, s_Instance->m_CurrentDirectory, "PBR");
        material->Set("u_Material.Albedo", glm::vec3(0.5f, 0.5f, 0.5f));
        // material->Set("u_AlbedoTex", "");
        material->Set("u_Material.Metalness", 0.5f);
        material->Set("u_Material.Roughness", 0.5f);
        material->Set("u_Material.Ambient", 1.0f);
        material->Set("u_Material.AmbientStrength", 0.1f);
        material->Set("u_Material.Specular", 0.5f);
        material->Set("u_Material.enableAlbedoTex", false);
        material->Set("u_Material.enableMetalnessTex", false);
        material->Set("u_Material.enableRoughnessTex", false);
        material->Set("u_Material.enableNormalTex", false);

        auto task = Ref<ThumbnailPoolTask>::Create(material, PoolTaskFlags_Export);
        Pool::AddTask(task);
        Pool::Start();
        s_Instance->OnBrowserRefresh();

        return material;
    }

    void ContentBrowserPanel::RenderAddNewContextMenu()
    {
        if (ImGui::BeginPopup("AddNewContextMenu"))
        {
            if (ImGui::MenuItem("Folder"))
            {
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Scene"))
            {
                CreateScene();
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Texture"))
            {
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Material"))
            {
                CreateMaterial();
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Mesh"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    void ContentBrowserPanel::RenderItemContextMenu()
    {
        if (ImGui::BeginPopup("ItemContextMenu"))
        {
            if (ImGui::MenuItem("Rename"))
            {
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Delete"))
            {
                auto items = m_ContentSelection.GetSelection();
                for (auto [handle, item] : items)
                {
                    item->Delete();
                }
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Properties"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    void ContentBrowserPanel::RenderAssetMenu(float height)
    {
        ImGui::BeginChild("##asset_menu", ImVec2(0, height));
        if (UI::IconButton("Add New", IM_COL32(70, 160, 0, 255), IM_COL32(70, 180, 40, 255), IM_COL32(100, 190, 40, 255)))
        {
            ImGui::OpenPopup("AddNewContextMenu");
        }
        RenderAddNewContextMenu();
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

        bool opened = ImGui::TreeNodeWithIcon(s_Icons["HierachyDirectory"], window->GetID(id.c_str()), flags, name.c_str(), NULL);

        if (ImGui::IsItemClicked())
            ChangeDirectory(directory);

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

                if (contentBrowserButton("##back", s_Icons["Back"], m_BackIcon_Disabled))
                    OnBrowserBack();

                if (contentBrowserButton("##forward", s_Icons["Back"], m_ForwardIcon_Disabled, true))
                    OnBrowserForward();

                if (contentBrowserButton("##refresh", s_Icons["Refresh"]))
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
                ImGui::Image((ImTextureID)(uintptr_t)s_Icons["Search"]->GetRendererID(), iconSize, ImVec2(0, 1), ImVec2(1, 0), ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
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

                    UI::DrawButtonImage(s_Icons["Clear"], IM_COL32(160, 160, 160, 200),
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
        fs::path path = FileDialogs::OpenFile("Import (*.png)\0*.jpeg\0");

        Texture2DSpecification spec;
        spec.WrapR = ImageParameter::CLAMP_TO_EDGE;
        spec.WrapS = ImageParameter::CLAMP_TO_EDGE;
        spec.WrapT = ImageParameter::CLAMP_TO_EDGE;

        Ref<Asset> textureAsset = CreateAsset<Texture2D>(path.filename().stem().string(), m_CurrentDirectory, path.string(), spec);

        SaveAllAssets();
        OnBrowserRefresh();
    }

    void ContentBrowserPanel::SaveAllAssets()
    {
        auto assetManager = Application::GetAssetManager();
        assetManager->SaveAssets();
        RenderItemThumbnails(m_CurrentItems);
    }

    void ContentBrowserPanel::RenderItemThumbnails(std::vector<Ref<ContentItem>> items)
    {
        auto assetManager = Application::GetAssetManager();
        for (auto item : items)
        {
            auto metadata = assetManager->GetMetadata(item->GetHandle());
            auto asset = assetManager->GetAsset(metadata.Handle);

            if (asset)
            {
                auto task = Ref<ThumbnailPoolTask>::Create(asset, PoolTaskFlags_Export);
                Pool::AddTask(task);
            }

        }

        // Cache the renderer ouput for MaterialPanel because it will change after thumbnails rendered.
        ThumbnailRenderer::GetRenderer<MaterialThumbnailRenderer>()->CreateCache();
        Pool::Start();
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

    AssetHandle ContentBrowserPanel::ProcessDirectory(const fs::path &directoryPath, const Ref<DirectoryInfo> &parent)
    {
		Ref<DirectoryInfo> directoryInfo = Ref<DirectoryInfo>::Create();
        directoryInfo->Handle = AssetHandle();
        directoryInfo->Parent = parent;

		if (directoryPath == Utils::File::GetAssetDirectory())
            directoryInfo->FilePath = "";
        else
			directoryInfo->FilePath = fs::relative(directoryPath, Utils::File::GetAssetDirectory());

		for (auto entry : fs::directory_iterator(directoryPath))
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
            {
                AddAssetsToDir(directoryInfo, metadata);
            }
        }

        SortSubDirs(directoryInfo);
        SortAssets(directoryInfo);

		m_Directories[directoryInfo->Handle] = directoryInfo;

        return directoryInfo->Handle;
    }

    void ContentBrowserPanel::DeleteItems(std::vector<Ref<ContentItem>> items)
    {
        bool shouldRefresh = items.size() > 0;
        for (auto item : items)
        {
            auto it = std::find_if(m_CurrentItems.begin(), m_CurrentItems.end(), [item](const Ref<ContentItem> i) {
                return i == item;
            });

            if (it != m_CurrentItems.end())
            {
                m_CurrentItems.erase(it);
                m_CurrentDirectory->Assets.erase(item->GetHandle());
                Application::GetAssetManager()->RemoveAsset(item->GetHandle());
                ThumbnailManager::DeleteThumbnail(item->GetHandle());
            }
        }

        if (shouldRefresh)
            OnBrowserRefresh();
    }

    std::string ContentBrowserPanel::CreateItemName(AssetType type)
    {
        auto itemType = Utils::AssetTypeToString(type);

        int maxIndex = -1;
        std::regex pattern(R"(material_(\d+))");
        std::smatch match;

        for (const auto& item : s_Instance->m_CurrentItems) {
            auto name = item->GetFilename();
            if (name == itemType) {
                maxIndex = std::max(maxIndex, 0);
            } else if (std::regex_search(name, match, pattern)) {
                int index = std::stoi(match[1]);
                maxIndex = std::max(maxIndex, index);
            }
        }

        std::string itemName = itemType;
        if (maxIndex > -1)
            itemName += ("_" + std::to_string(maxIndex + 1));

        return itemName;
    }
}
