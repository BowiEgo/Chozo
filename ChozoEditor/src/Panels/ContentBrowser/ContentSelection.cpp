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
            m_Disabled = false;
            m_Dragging = false;
            m_StartPos = io.MousePos;
            m_EndPos = m_StartPos;

            if (ContentBrowserPanel::GetHoveredItem())
                m_Disabled = true;
        }
        else if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            m_EndPos = io.MousePos;

            float deltaX = m_EndPos.x - m_StartPos.x;
            float deltaY = m_EndPos.y - m_StartPos.y;
            float distance = sqrtf(deltaX * deltaX + deltaY * deltaY);

            if (distance > selectionThreshold && !m_Disabled)
            {
                m_Dragging = true;
            }
        }
        else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            m_Dragging = false;
        }

        ImVec2 rectMin, rectMax;

        // Draw the selection box if dragging
        if (m_Dragging)
        {
            rectMin = ImVec2(fminf(m_StartPos.x, m_EndPos.x), fminf(m_StartPos.y, m_EndPos.y));
            rectMax = ImVec2(fmaxf(m_StartPos.x, m_EndPos.x), fmaxf(m_StartPos.y, m_EndPos.y));
            UI::DrawDashedRect(rectMin, rectMax, IM_COL32(150, 150, 150, 255));
        }

        auto items = ContentBrowserPanel::GetItems();
        for (size_t i = 0; i < items.size(); i++)
        {
            auto itemRect = items[i]->GetRect();
            
            if (itemRect.Min.x == 0 && itemRect.Min.y == 0 && itemRect.Max.x == 0 && itemRect.Max.y == 0)
                break;

            bool intersecting = !(itemRect.Min.x > rectMax.x || itemRect.Max.x < rectMin.x || itemRect.Min.y > rectMax.y || itemRect.Max.y < rectMin.y);

            if (intersecting)
                items[i]->Select();
            else if (m_Dragging)
                items[i]->Deselect();
        }
    }
} // namespace Chozo
