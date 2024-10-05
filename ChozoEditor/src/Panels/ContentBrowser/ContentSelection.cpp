#include "ContentSelection.h"

#include "ContentBrowserPanel.h"

#include "Chozo/ImGui/ImGuiUI.h"

namespace Chozo {

    ContentSelection::ContentSelection()
    {
    }

    ContentSelection::~ContentSelection()
    {
    }

    void ContentSelection::OnImGuiRender()
    {
        ImGuiIO& io = ImGui::GetIO();
        const float selectionThreshold = 10.0f;

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            m_Selecting = false;
            m_StartPos = io.MousePos;
            m_EndPos = m_StartPos;
        }
        else if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            m_EndPos = io.MousePos;

            float deltaX = m_EndPos.x - m_StartPos.x;
            float deltaY = m_EndPos.y - m_StartPos.y;
            float distance = sqrtf(deltaX * deltaX + deltaY * deltaY);

            if (distance > selectionThreshold)
            {
                m_Selecting = true;
            }
        }
        else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            m_Selecting = false;
        }

        ImVec2 rectMin, rectMax;
        // Step 2: Draw the selection box if dragging
        if (m_Selecting)
        {
            rectMin = ImVec2(fminf(m_StartPos.x, m_EndPos.x), fminf(m_StartPos.y, m_EndPos.y));
            rectMax = ImVec2(fmaxf(m_StartPos.x, m_EndPos.x), fmaxf(m_StartPos.y, m_EndPos.y));
            UI::DrawDashedRect(rectMin, rectMax, IM_COL32(150, 150, 150, 255));
        }

        auto items = ContentBrowserPanel::GetItems();
        for (size_t i = 0; i < items.size(); i++)
        {
            auto [itemMin, itemMax] = items[i]->GetRect();

            bool intersecting = !(itemMin.x > rectMax.x || itemMax.x < rectMin.x || itemMin.y > rectMax.y || itemMax.y < rectMin.y);

            if (intersecting)
                Push(items[i]);
            else if (m_Selecting)
                Deselect(items[i]);
        }
    }

    void ContentSelection::Clear()
    {
        for (auto [handle, item] : m_Selection)
        {
            m_Selection[handle]->Deselect();
        }

        m_Selection.clear();
    }

    void ContentSelection::Push(Ref<ContentItem> item)
    {
        auto handle = item->GetHandle();
        if (m_Selection.find(handle) == m_Selection.end())
        {
            m_Selection[handle] = item;
            item->Select();
        }
    }

    void ContentSelection::Select(Ref<ContentItem> item)
    {
        Clear();
        Push(item);
    }

    void ContentSelection::Deselect(Ref<ContentItem> item)
    {
        auto handle = item->GetHandle();
        Deselect(handle);
    }

    void ContentSelection::Deselect(AssetHandle handle)
    {
        if (m_Selection.find(handle) != m_Selection.end())
        {
            m_Selection[handle]->Deselect();
            m_Selection.erase(handle);
        }
    }

} // namespace Chozo
