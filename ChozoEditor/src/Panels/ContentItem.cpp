#include "ContentItem.h"
#include "ContentBrowserPanel.h"

#include "Chozo/ImGui/ImGuiUI.h"

namespace Chozo {

    static void RenderThumbnail(const Ref<Texture2D>& icon, float thumbnailSize)
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

    static void RenderCenteredText(const std::string& text)
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

    static void RenderDragDrop(const AssetHandle& handle)
    {
        if (ImGui::BeginDragDropSource())
        {
            std::wstring handle_wstr = std::to_wstring(handle);
            const wchar_t* handle_wchar = handle_wstr.c_str();
            ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", handle_wchar, (wcslen(handle_wchar) + 1) * sizeof(wchar_t));
            ImGui::EndDragDropSource();
        }
    }

    ContentItem::ContentItem(Ref<DirectoryInfo> directory)
    {
        m_Type = ContentItemType::Directory;
        m_Handle = directory->Handle;
        m_Filename = directory->FilePath.filename().string();
        m_Thumbnail = directory->SubDirectories.empty() ? ContentBrowserPanel::GetIcon("EmptyDirectory") : ContentBrowserPanel::GetIcon("Directory");
    }

    ContentItem::ContentItem(AssetMetadata metadata)
    {
        m_Type = ContentItemType::Asset;
        m_Handle = metadata.Handle;
        m_Filename = metadata.FilePath.filename().string();
        m_Size = metadata.FileSize;

        if (metadata.Type == AssetType::Texture)
            m_Thumbnail = ContentBrowserPanel::Get().GetThumbnailManager()->GetThumbnail(metadata.Handle);
        else
            m_Thumbnail = ContentBrowserPanel::GetIcon("TextFile");
    }

    void ContentItem::OnImGuiRender()
    {
        UI::ScopedID id(m_Filename.c_str());

        RenderThumbnail(m_Thumbnail, ContentBrowserPanel::s_ThumbnailSize);
        RenderTooltip();
        RenderDragDrop(m_Handle);
        OnDoubleClick();
        RenderCenteredText(m_Filename);
    }

    void ContentItem::RenderTooltip()
    {
        if (ImGui::BeginItemTooltip())
        {
            ImGui::Text("Handle: %s", std::to_string(m_Handle).c_str());
            ImGui::Text("Size: %s", Utils::File::BytesToHumanReadable(m_Size).c_str());
            ImGui::EndTooltip();
        }
    }

    void ContentItem::OnDoubleClick()
    {
        if (m_Type == ContentItemType::Directory && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            ContentBrowserPanel::Get().ChangeDirectory(m_Handle);
    }
}