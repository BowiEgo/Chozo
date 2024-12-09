#include "ContentItem.h"
#include "ContentBrowserPanel.h"
#include "Panels/MaterialPanel.h"
#include "Thumbnail/ThumbnailManager.h"

namespace Chozo {

    ContentItem::ContentItem(Ref<DirectoryInfo> directory)
    {
        m_Type = ContentItemType::Directory;
        m_Handle = directory->Handle;
        m_Filename = directory->FilePath.filename().string();
        m_Empty = directory->SubDirectories.empty() && directory->Assets.empty();
    }

    ContentItem::ContentItem(const AssetMetadata& metadata)
    {
        m_Type = ContentItemType::Asset;
        m_AssetType = metadata.Type;
        m_Handle = metadata.Handle;
        m_Filename = metadata.FilePath.filename().string();
        m_Size = metadata.FileSize;
        m_CreateAt = metadata.CreateAt;
    }

    ContentBrowserItemActionResult ContentItem::OnImGuiRender()
    {
        ContentBrowserItemActionResult result;

        ImVec2 textSize = ImGui::CalcTextSize(m_Filename.c_str());
        ImVec2 thumbnailSize = ImVec2(ContentBrowserPanel::s_ThumbnailSize, ContentBrowserPanel::s_ThumbnailSize);
        ImVec2 itemSize = ImVec2(thumbnailSize.x, thumbnailSize.y + textSize.y + ImGui::GetStyle().ItemSpacing.y);

        m_Rect.Min = ImGui::GetCursorScreenPos();
        m_Rect.Max = m_Rect.Min + itemSize;

        UI::ScopedID id(m_Handle);
        if (ImGui::Selectable("##Selectable", m_Selected, ImGuiSelectableFlags_None, itemSize))
        {
            if (m_AssetType == AssetType::Material)
            {
                auto material = Application::GetAssetManager()->GetAsset(m_Handle);
                MaterialPanel::SetMaterial(material);
                MaterialPanel::Open();
            }
        }

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            result.Set(ContentBrowserAction::Activated, true);
        }

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
        {
            result.Set(ContentBrowserAction::Hovered, true);
        }

        RenderDragDrop(m_Handle);
        ImVec2 cursorPos = ImGui::GetCursorScreenPos();
        cursorPos.y = cursorPos.y - itemSize.y - 6.0f;
        ImGui::SetCursorScreenPos(cursorPos);
        RenderThumbnail();
        RenderTooltip();
        RenderCenteredText(m_Filename);

        if (m_Selected)
        {
            // ImGui::GetWindowDrawList()->AddRect(rectMin, rectMax, IM_COL32(150, 150, 150, 255));
            // PropertiesPanel::Get().SetSelectedEntity({});
            result.Set(ContentBrowserAction::Selected, true);
        }

        if (m_Delete)
            result.Set(ContentBrowserAction::Deleted, true);

        return result;
    }

    void ContentItem::UpdateThumbnail()
    {
        if (m_Type == ContentItemType::Directory)
            m_Thumbnail = m_Empty ? ContentBrowserPanel::GetIcon("EmptyDirectory") : ContentBrowserPanel::GetIcon("Directory");
        else
        {
            switch (m_AssetType)
            {
                case AssetType::Scene:
                case AssetType::Texture:
                case AssetType::Material:
                    m_Thumbnail = ThumbnailManager::GetThumbnail(m_Handle);
                    break;
                default:
                    m_Thumbnail = ContentBrowserPanel::GetIcon("TextFile");
                    break;
            }
        }
    }

    void ContentItem::RenderThumbnail()
    {
        UpdateThumbnail();

        if (!m_Thumbnail)
        {
            auto asset = Application::GetAssetManager()->GetAsset(m_Handle);
            if (asset)
            {
                auto task = Ref<ThumbnailPoolTask>::Create(asset, PoolTaskFlags_Export);
                Application::Get().GetPool()->AddTask(task);

                // Cache the renderer ouput for MaterialPanel because it will change after thumbnails rendered.
                ThumbnailRenderer::GetRenderer<MaterialThumbnailRenderer>()->CreateCache();
                Application::Get().GetPool()->Start();
            }
        }

        auto thumbnail = m_Thumbnail ? m_Thumbnail : Renderer::GetCheckerboardTexture();

        float thumbnailSize = ContentBrowserPanel::s_ThumbnailSize;
        float imageAspectRatio = static_cast<float>(thumbnail->GetHeight()) / static_cast<float>(thumbnail->GetWidth());
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
        UI::DrawButtonImage(thumbnail, IM_COL32(255, 255, 255, 225), UI::RectExpanded(UI::GetItemRect(), -6.0f, -6.0f), uv0, uv1);

        ImGui::PopStyleColor();

        OnDoubleClick();
    }

    void ContentItem::RenderTooltip()
    {
        if (ImGui::BeginItemTooltip())
        {
            auto size = Utils::File::BytesToHumanReadable(m_Size);
            if (size.find("Bytes") == std::string::npos)
                size += (" (" + Utils::Numeric::FormatWithCommas(m_Size) + "Bytes)");

            ImGui::Text("ID: %s", std::to_string(m_Handle).c_str());
            ImGui::Text("Disk Space: %s", size.c_str());
            ImGui::Text("CreatAt: %s", Utils::Time::FormatTimestamp(m_CreateAt).c_str());
            ImGui::EndTooltip();
        }
    }

    void ContentItem::RenderDragDrop(const UUID& handle)
    {
        if (ImGui::BeginDragDropSource())
        {
            std::wstring handle_wstr = std::to_wstring(handle);
            const wchar_t* handle_wchar = handle_wstr.c_str();
            ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", handle_wchar, (wcslen(handle_wchar) + 1) * sizeof(wchar_t));
            ImGui::EndDragDropSource();
        }
    }

    void ContentItem::RenderCenteredText(const std::string& text)
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

    void ContentItem::OnDoubleClick() const {
        if (m_Type == ContentItemType::Directory && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            ContentBrowserPanel::Get().ChangeDirectory(m_Handle);
    }
}