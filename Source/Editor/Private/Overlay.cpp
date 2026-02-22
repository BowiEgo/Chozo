#include "Overlay.h"

Overlay::Overlay() {}

Overlay::~Overlay() {}

void Overlay::Draw(bool* bIsOpen, const std::function<void()>& contentRenderFn) {
    if (!*bIsOpen) return;

    // English Comment: 1. External Padding (Positioning)
    ImVec2 parent_pos = ImGui::GetWindowPos();
    ImVec2 parent_size = ImGui::GetWindowSize();
    float edge_pad = 10.0f * ImGui::GetIO().FontGlobalScale;

    ImVec2 window_pos, window_pos_pivot;

    if (m_LocationIndex >= 0) {
        window_pos.x = (m_LocationIndex & 1) ? (parent_pos.x + parent_size.x - edge_pad)
                                             : (parent_pos.x + edge_pad);
        window_pos.y = (m_LocationIndex & 2) ? (parent_pos.y + parent_size.y - edge_pad)
                                             : (parent_pos.y + edge_pad);

        window_pos_pivot.x = (m_LocationIndex & 1) ? 1.0f : 0.0f;
        window_pos_pivot.y = (m_LocationIndex & 2) ? 1.0f : 0.0f;

        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    } else if (m_LocationIndex == -2) {
        // Center relative to the parent window
        window_pos =
            ImVec2(parent_pos.x + parent_size.x * 0.5f, parent_pos.y + parent_size.y * 0.5f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    }
    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking |
                                    ImGuiWindowFlags_AlwaysAutoResize |
                                    ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    float fixedWidth = 360.0f;
    ImGui::SetNextWindowSizeConstraints(ImVec2(fixedWidth, -1.0f), ImVec2(fixedWidth, 1000.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f);
    ImGui::SetNextWindowBgAlpha(0.35f);
    if (ImGui::Begin("Overlay:", bIsOpen, window_flags)) {
        // ImGui::Text("Status: (right-click to change position)");
        // ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 2.0f));
        ImGui::PushTextWrapPos(fixedWidth - 10.0f);
        contentRenderFn();
        ImGui::PopTextWrapPos();

        if (ImGui::BeginPopupContextWindow()) {
            if (ImGui::MenuItem("Custom", NULL, m_LocationIndex == -1)) m_LocationIndex = -1;
            if (ImGui::MenuItem("Center", NULL, m_LocationIndex == -2)) m_LocationIndex = -2;
            if (ImGui::MenuItem("Top-left", NULL, m_LocationIndex == 0)) m_LocationIndex = 0;
            if (ImGui::MenuItem("Top-right", NULL, m_LocationIndex == 1)) m_LocationIndex = 1;
            if (ImGui::MenuItem("Bottom-left", NULL, m_LocationIndex == 2)) m_LocationIndex = 2;
            if (ImGui::MenuItem("Bottom-right", NULL, m_LocationIndex == 3)) m_LocationIndex = 3;
            if (bIsOpen && ImGui::MenuItem("Close")) *bIsOpen = false;
            ImGui::EndPopup();
        }
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
}

void Overlay::UpdateLocation(EOverlayLocation Location) {
    switch (Location) {
    case EOverlayLocation::TopLeft:
        m_LocationIndex = 0;
        break;
    case EOverlayLocation::TopRight:
        m_LocationIndex = 1;
        break;
    case EOverlayLocation::BottomLeft:
        m_LocationIndex = 2;
        break;
    case EOverlayLocation::BottomRight:
        m_LocationIndex = 3;
        break;
    case EOverlayLocation::Center:
        m_LocationIndex = -2;
        break;
    default:
        m_LocationIndex = 0;
        break;
    }
}
