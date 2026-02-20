#include "EditorLayer.h"

#include "Application.h"

#include "imgui.h"

EditorLayer::EditorLayer() : ILayer("Editor") {}

void EditorLayer::OnAttach() {
    m_ViewportRenderer = CApplication::Get()->GetRenderEngine()->GetRenderer();
}
void EditorLayer::OnDetach() {}
void EditorLayer::OnUpdate(FTimeStep ts) {}
void EditorLayer::OnImGuiRender() {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
    //-------------------- Dockspace begin -------------------------------------
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar();

    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();
    float minWinSizeX = style.WindowMinSize.x;
    style.WindowMinSize.x = 300.0f;
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    }
    style.WindowMinSize.x = minWinSizeX;

    // --------------------
    // Viewport
    // --------------------
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
    ImGui::Begin("Viewport");

    // m_ViewportFocused = ImGui::IsWindowFocused();
    // m_ViewportHovered = ImGui::IsWindowHovered();
    // Application::Get().GetImGuiLayer().BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

    auto viewportOffset = ImGui::GetCursorPos(); // includes tab bar
    m_ViewportSize = ImGui::GetContentRegionAvail();

    auto texture = m_ViewportRenderer->GetSceneFrameBuffer()->GetColorAttachment(0);
    ImTextureID textureID = (ImTextureID)texture->GetDescriptorSet();
    ImGui::Image(textureID, m_ViewportSize, ImVec2(0, 1), ImVec2(1, 0));

    ImGui::End();
    ImGui::PopStyleVar();

    //-------------------- Dockspace end -------------------------------------
    ImGui::End();
}
void EditorLayer::OnEvent(IEvent& e) {}