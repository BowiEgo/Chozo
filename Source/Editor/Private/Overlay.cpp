#include "Overlay.h"

Overlay::Overlay() {}

Overlay::~Overlay() {}

void Overlay::OnImGuiRender(bool* bIsOpen, const std::function<void()>& contentRenderFn) {
    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking |
                                    ImGuiWindowFlags_AlwaysAutoResize |
                                    ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    if (m_LocationIndex >= 0) {
        const float PAD = 10.0f;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
        ImVec2 work_size = viewport->WorkSize;
        ImVec2 window_pos, window_pos_pivot;
        window_pos.x = (m_LocationIndex & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
        window_pos.y = (m_LocationIndex & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
        window_pos_pivot.x = (m_LocationIndex & 1) ? 1.0f : 0.0f;
        window_pos_pivot.y = (m_LocationIndex & 2) ? 1.0f : 0.0f;
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        ImGui::SetNextWindowViewport(viewport->ID);
        window_flags |= ImGuiWindowFlags_NoMove;
    } else if (m_LocationIndex == -2) {
        // Center window
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always,
                                ImVec2(0.5f, 0.5f));
        window_flags |= ImGuiWindowFlags_NoMove;
    }
    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    if (ImGui::Begin("Overlay:", bIsOpen, window_flags)) {
        ImGui::Text("Status: (right-click to change position)");
        ImGui::Separator();

        contentRenderFn();

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
}

void Overlay::UpdateLocation(EOverlayLocation Location) {
    switch (Location) {
    case EOverlayLocation::TopLeft:     m_LocationIndex = 0; break;
    case EOverlayLocation::TopRight:    m_LocationIndex = 1; break;
    case EOverlayLocation::BottomLeft:  m_LocationIndex = 2; break;
    case EOverlayLocation::BottomRight: m_LocationIndex = 3; break;
    case EOverlayLocation::Center:      m_LocationIndex = -2; break;
    default:                            m_LocationIndex = 0; break;
    }
}
