#include "Overlay.h"

UOverlay::UOverlay() {}

UOverlay::~UOverlay() {}

void UOverlay::Draw(const char* title, bool* bIsOpen,
                    const std::function<void()>& contentRenderFn) {
    if (!*bIsOpen) return;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Get Viewport metrics
    ImVec2 parentPos = ImGui::GetWindowPos();
    ImVec2 parentSize = ImGui::GetWindowSize();
    ImVec2 contentMin = ImGui::GetWindowContentRegionMin();

    float edgePad = 10.0f * ImGui::GetIO().FontGlobalScale;
    float padding = 10.0f;

    float boxWidth = m_LastSize.x + padding * 2.0f;
    boxWidth = boxWidth < 360.0f ? 360.0f : boxWidth;
    // Calculate Start Position based on the size from THE PREVIOUS FRAME
    // This ensures we can draw the background BEFORE the text in a single pass.
    ImVec2 boxSize = { boxWidth, m_LastSize.y + padding * 2.0f };
    ImVec2 startPos;

    if (m_LocationIndex == -2) { // Center
        startPos.x = parentPos.x + (parentSize.x - boxSize.x) * 0.5f;
        startPos.y = parentPos.y + (parentSize.y - boxSize.y) * 0.5f;
    } else {
        // X: 1 is Right, 0 is Left
        startPos.x = (m_LocationIndex & 1) ? (parentPos.x + parentSize.x - boxSize.x - edgePad)
                                           : (parentPos.x + edgePad);
        // Y: 2 is Bottom, 0 is Top
        startPos.y = (m_LocationIndex & 2) ? (parentPos.y + parentSize.y - boxSize.y - edgePad)
                                           : (parentPos.y + contentMin.y + edgePad);
    }

    // Draw Background first (Now we have the position!)
    ImVec2 rectMax = { startPos.x + boxSize.x, startPos.y + boxSize.y };
    draw_list->AddRectFilled(startPos, rectMax, IM_COL32(20, 20, 20, 120), 6.0f);
    draw_list->AddRect(startPos, rectMax, IM_COL32(150, 150, 150, 80), 6.0f, 0, 1.2f);

    // IMPORTANT - Force Cursor to startPos before rendering text
    // Without this, ImGui::Text() will draw at (0,0) or wherever the cursor was left.
    ImGui::SetCursorScreenPos(startPos + ImVec2(padding, padding));

    // Render Content and measure its size for the next frame
    ImGui::BeginGroup();
    contentRenderFn();
    ImGui::EndGroup();

    // Update size for the next frame to handle dynamic content
    m_LastSize = ImGui::GetItemRectSize();
}

void UOverlay::UpdateLocation(EOverlayLocation Location) {
    switch (Location) {
        case EOverlayLocation::TopLeft: m_LocationIndex = 0; break;
        case EOverlayLocation::TopRight: m_LocationIndex = 1; break;
        case EOverlayLocation::BottomLeft: m_LocationIndex = 2; break;
        case EOverlayLocation::BottomRight: m_LocationIndex = 3; break;
        case EOverlayLocation::Center: m_LocationIndex = -2; break;
        default: m_LocationIndex = 0; break;
    }
}
