#include "ContentBrowserPanel.h"

#include <regex>

#include "Chozo/ImGui/CustomTreeNode.h"
#include "Chozo/ImGui/ImGuiUI.h"
#include "Chozo/ImGui/Colors.h"

#include "Chozo/Utilities/PlatformUtils.h"
#include "Chozo/Renderer/Material.h"
#include "Chozo/Core/Pool.h"

#include "Thumbnail/ThumbnailManager.h"
#include "Thumbnail/ThumbnailRenderer.h"
#include "Thumbnail/ThumbnailPoolTask.h"
#include <imgui_internal.h>

namespace Chozo {

	float ContentBrowserPanel::s_Padding = 16.0f;
	float ContentBrowserPanel::s_ThumbnailSize = 100.0f;
    ContentBrowserPanel* ContentBrowserPanel::s_Instance = nullptr;

    ContentBrowserPanel::ContentBrowserPanel()
    {
        s_Instance = this;

        m_Icons["HierarchyDirectory"] = Texture2D::Create(std::string("../resources/icons/ContentBrowser/folder-open-1.png"));
        m_Icons["Directory"] = Texture2D::Create(std::string("../resources/icons/ContentBrowser/folder.png"));
        m_Icons["EmptyDirectory"] = Texture2D::Create(std::string("../resources/icons/ContentBrowser/folder-empty.png"));
        m_Icons["TextFile"] = Texture2D::Create(std::string("../resources/icons/ContentBrowser/file.png"));
        // Toolbar
        m_Icons["Back"] = Texture2D::Create(std::string("../resources/icons/ContentBrowser/left-arrow.png"));
        m_Icons["Refresh"] = Texture2D::Create(std::string("../resources/icons/ContentBrowser/refresh.png"));
        m_Icons["Search"] = Texture2D::Create(std::string("../resources/icons/ContentBrowser/search.png"));
        m_Icons["Clear"] = Texture2D::Create(std::string("../resources/icons/ContentBrowser/clear.png"));

        m_ContentSelection = ContentSelection();
        OnBrowserRefresh();
    }

    void ContentBrowserPanel::OnImGuiRender()
    {
        m_ItemsShouldDelete.clear();
        m_ItemsSelected.clear();

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

                RenderItems();
                RenderItemContextMenu();
                m_ContentSelection.OnImGuiRender();
                if (m_HoveredItem)
                {
                    if (ImGui::IsMouseClicked(0))
                    {
                        ImGui::CloseCurrentPopup();
                        for (auto item : m_ItemsSelected)
                            item->Deselect();
    
                        m_HoveredItem->Select();
                    }

                    if (ImGui::IsMouseClicked(1))
                    {
                        if (std::find(m_ItemsSelected.begin(), m_ItemsSelected.end(), m_HoveredItem) == m_ItemsSelected.end())
                        {
                            for (auto item : m_ItemsSelected)
                                item->Deselect();

                            m_HoveredItem->Select();
                        }
                        ImGui::OpenPopup("ItemContextMenu");
                    }
                }
                else
                {
                    if (ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered())
                    {
                        if (!ImGui::IsPopupOpen("ItemContextMenu"))
                        {
                            for (auto item : m_ItemsSelected)
                                item->Deselect();
                        }

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

        // Delete items
        DeleteItems(m_ItemsShouldDelete);
    }

    void ContentBrowserPanel::ChangeDirectory(Ref<DirectoryInfo>& directory)
    {
        m_NextDirectory = nullptr;
        m_PreviousDirectory = m_CurrentDirectory;
        m_CurrentDirectory = directory;

        OnDirectoryChange();
    }

    void ContentBrowserPanel::ChangeDirectory(AssetHandle directoryHandle)
    {
        ChangeDirectory(m_Directories[directoryHandle]);
    }

    void ContentBrowserPanel::OnDirectoryChange()
    {
        if (!m_CurrentDirectory)
            return;

        m_CurrentItems.clear();

        for (auto& [subdirHandle, subDir] : m_CurrentDirectory->SubDirectories)
            m_CurrentItems.push_back(Ref<ContentItem>::Create(subDir));

        for (auto& [AssetHandle, metadata] : m_CurrentDirectory->Assets)
            m_CurrentItems.push_back(Ref<ContentItem>::Create(metadata));
    }

    void ContentBrowserPanel::OnBrowserBack()
    {
        if (!m_BackIcon_Disabled)
        {
		    m_NextDirectory = m_CurrentDirectory;
    		m_PreviousDirectory = m_CurrentDirectory->Parent;
            m_CurrentDirectory = m_PreviousDirectory;
            OnDirectoryChange();
        }
    }

    void ContentBrowserPanel::OnBrowserForward()
    {
        m_CurrentDirectory = m_NextDirectory;
        OnDirectoryChange();
    }

    void ContentBrowserPanel::OnBrowserRefresh()
    {
        AssetHandle baseDirectoryHandle = ProcessDirectory(Utils::File::GetAssetDirectory(), nullptr);
        m_BaseDirectory = m_Directories[baseDirectoryHandle];
        OnDirectoryChange();
    }

    Ref<Scene> ContentBrowserPanel::CreateScene()
    {
        if (!s_Instance->m_CurrentDirectory)
        {
            CZ_CORE_ERROR("ContentBrowser: Current directory is not exist!");
            return nullptr;
        }

        const auto filename = CreateItemName(AssetType::Scene);

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

        const auto filename = CreateItemName(AssetType::Material);

        auto material = s_Instance->CreateAsset<Material>(filename, s_Instance->m_CurrentDirectory, filename);

        s_Instance->OnBrowserRefresh();

        auto item = s_Instance->GetItem(material->Handle);
        item->Select();
        item->OpenMaterialPanel();

        const auto task = Ref<ThumbnailPoolTask>::Create(material, PoolTaskFlags_Export);

        Application::Get().GetPool()->AddTask(task);
        Application::Get().GetPool()->Start();

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

    void ContentBrowserPanel::RenderItems()
    {
        const float cellSize = s_ThumbnailSize + s_Padding;

        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = static_cast<int>(panelWidth / cellSize);
        columnCount = columnCount < 1 ? 1 : columnCount;

        ImGui::Columns(columnCount, nullptr, false);

        if (m_CurrentDirectory)
        {
            m_HoveredItem = nullptr;
            for (auto item : m_CurrentItems)
            {
                auto actionResult = item->OnImGuiRender();

                if (actionResult.IsSet(ContentBrowserAction::Deleted))
                {
                    m_ItemsShouldDelete.push_back(item);
                    break;
                }

                if (actionResult.IsSet(ContentBrowserAction::Activated))
                {
                    if (item->m_Type == ContentItemType::Directory)
                    {
                        ChangeDirectory(item->m_Handle);
                        break;
                    }
                }

                if (actionResult.IsSet(ContentBrowserAction::Hovered))
                {
                    m_HoveredItem = item;
                }

                if (actionResult.IsSet(ContentBrowserAction::Selected))
                {
                    m_ItemsSelected.push_back(item);
                }

                
                ImGui::NextColumn();
            }
        }

        ImGui::Columns(1);
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
                for (auto item : m_ItemsSelected)
                    item->Delete();
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

        const bool opened = ImGui::TreeNodeWithIcon(m_Icons["HierarchyDirectory"], window->GetID(id.c_str()), flags, name.c_str(), nullptr);

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
			// const float edgeOffset = 4.0f;
            // Navigation buttons
            {
                UI::ScopedStyle spacing(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 0.0f));

                auto contentBrowserButton = [height](const char* labelId, const Ref<Texture2D>& icon, bool isDisabled = false, bool isFlipped = false)
                {
					// const ImU32 buttonCol = Colors::Theme::backgroundDark;
                    // const ImU32 buttonColP = UI::ColourWithMultipliedValue(Colors::Theme::backgroundDark, 0.8f);
                    
                    const float iconSize = std::min(24.0f, height);
                    constexpr float iconPadding = 2.0f;
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

                if (contentBrowserButton("##back", m_Icons["Back"], m_BackIcon_Disabled))
                    OnBrowserBack();

                if (contentBrowserButton("##forward", m_Icons["Back"], m_ForwardIcon_Disabled, true))
                    OnBrowserForward();

                if (contentBrowserButton("##refresh", m_Icons["Refresh"]))
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
            {
                const ImVec2 iconSize(ImGui::GetTextLineHeight() - 2.0f, ImGui::GetTextLineHeight() - 2.0f);
                const float iconYOffset = framePaddingY;
				UI::ShiftCursorY(iconYOffset);
                ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(m_Icons["Search"]->GetRendererID())), iconSize, ImVec2(0, 1), ImVec2(1, 0), ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
				UI::ShiftCursorY(-iconYOffset);
                ImGui::SameLine();

                // Hint
                if (!focused)
                {
                    UI::ShiftCursorY(-framePaddingY + 1.0f);
                    // UI::ScopedStyle padding(ImGuiStyleVar_FramePadding, ImVec2(0.0f, framePaddingY));
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

                    UI::DrawButtonImage(m_Icons["Clear"], IM_COL32(160, 160, 160, 200),
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
        AssetType type = Utils::GetAssetTypeFromExtension(path.extension().string());

        Texture2DSpecification textureSpec;
        textureSpec.WrapR = ImageParameter::CLAMP_TO_EDGE;
        textureSpec.WrapS = ImageParameter::CLAMP_TO_EDGE;
        textureSpec.WrapT = ImageParameter::CLAMP_TO_EDGE;

        switch (type)
        {
            case AssetType::Texture:
                CreateAsset<Texture2D>(path.filename().stem().string(), m_CurrentDirectory, path.string(), textureSpec);
                break;
            case AssetType::MeshSource:
                CreateAsset<MeshSource>(path.filename().stem().string(), m_CurrentDirectory, path.string());
                break;
            default:
                break;
        }

        OnBrowserRefresh();
    }

    void ContentBrowserPanel::SaveAllAssets()
    {
        auto assetManager = Application::GetAssetManager();
        const auto assetsModified = assetManager->GetAssetsModified();
        assetManager->SaveAssets();
        OnBrowserRefresh();
        RenderAssetThumbnails(assetsModified);
    }

    void ContentBrowserPanel::RenderAssetThumbnails(const std::vector<AssetMetadata>& metadatas)
    {
        for (const auto& metadata : metadatas)
        {
            auto asset = Application::GetAssetManager()->GetAsset(metadata.Handle);
            auto task = Ref<ThumbnailPoolTask>::Create(asset, PoolTaskFlags_Export);
            Application::Get().GetPool()->AddTask(task);
        }
        Application::Get().GetPool()->Start();
    }

    void ContentBrowserPanel::AddAssetToDir(Ref<DirectoryInfo> directory, const AssetMetadata& metadata)
    {
        directory->Assets[metadata.Handle] = metadata;
        directory->AssetsSorted.push_back(metadata.Handle);
    }

    void ContentBrowserPanel::RemoveAssetFromDir(Ref<DirectoryInfo> directory, AssetHandle handle)
    {
        directory->Assets.erase(handle);

        auto it = std::find(directory->AssetsSorted.begin(), directory->AssetsSorted.end(), handle);
        if (it != directory->AssetsSorted.end())
            directory->AssetsSorted.erase(it);
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

		for (const auto& entry : fs::directory_iterator(directoryPath))
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
                AddAssetToDir(directoryInfo, metadata);
            }
            else
            {
                auto handle = Application::GetAssetManager()->LoadAsset(entry.path());
                metadata = Application::GetAssetManager()->GetMetadata(handle);
                if (metadata.IsValid())
                    AddAssetToDir(directoryInfo, metadata);
            }
        }

        SortSubDirs(directoryInfo);
        SortAssets(directoryInfo);

		m_Directories[directoryInfo->Handle] = directoryInfo;

        if (m_CurrentDirectory)
        {
            if (directoryInfo->FilePath == m_CurrentDirectory->FilePath)
                m_CurrentDirectory = directoryInfo;
        }

        return directoryInfo->Handle;
    }

    Ref<ContentItem> ContentBrowserPanel::GetItem(AssetHandle handle)
    {
        const auto it = std::find_if(m_CurrentItems.begin(), m_CurrentItems.end(), [handle](const Ref<ContentItem>& i) {
            return i->GetHandle() == handle;
        });

        if (it != m_CurrentItems.end())
            return *it;

        return nullptr;
    }

    void ContentBrowserPanel::DeleteItem(Ref<ContentItem> item)
    {
        auto it = std::find_if(m_CurrentItems.begin(), m_CurrentItems.end(), [item](const Ref<ContentItem>& i) {
            return i == item;
        });

        if (it != m_CurrentItems.end())
        {
            m_CurrentItems.erase(it);

            RemoveAssetFromDir(m_CurrentDirectory, item->GetHandle());
            Application::GetAssetManager()->RemoveAsset(item->GetHandle());
            ThumbnailManager::DeleteThumbnail(item->GetHandle());
        }
    }

    void ContentBrowserPanel::DeleteItems(const std::vector<Ref<ContentItem>>& items)
    {
        for (auto item : items)
        {
            auto it = std::find_if(m_CurrentItems.begin(), m_CurrentItems.end(), [item](const Ref<ContentItem>& i) {
                return i == item;
            });

            if (it != m_CurrentItems.end())
            {
                m_CurrentItems.erase(it);

                RemoveAssetFromDir(m_CurrentDirectory, item->GetHandle());
                Application::GetAssetManager()->RemoveAsset(item->GetHandle());
                ThumbnailManager::DeleteThumbnail(item->GetHandle());
            }
        }
    }

    std::string ContentBrowserPanel::CreateItemName(AssetType type)
    {
        std::string result;
        std::string defaultName = std::string(Utils::AssetTypeToString(type));

        std::regex pattern(defaultName + R"(_(\d+))");
        std::smatch match;
        std::vector<int> indices;

        for (const auto& item : s_Instance->m_CurrentItems)
        {
            auto name = item->GetFilename();
            if (name == defaultName)
                indices.push_back(0);
            if (std::regex_search(name, match, pattern))
                indices.push_back(std::stoi(match[1].str()));
        }

        std::sort(indices.begin(), indices.end());
        int newIndex = 0;
        for (int index: indices)
        {
            if (index == newIndex)
                newIndex++;
            else if (index > newIndex)
                break;
        }

        result = newIndex == 0 ? defaultName : defaultName + "_" + std::to_string(newIndex);

        return result;
    }
}
