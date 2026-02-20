#include "EditorLayer.h"

#include "imgui.h"

EditorLayer::EditorLayer() {}

void EditorLayer::OnAttach() {}
void EditorLayer::OnDetach() {}
void EditorLayer::OnUpdate(FTimeStep ts) {}
void EditorLayer::OnImGuiRender() {
    ImGui::ShowDemoWindow();

    ImGui::Begin("Settings");
    ImGui::Text("Renderer stats:");
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    ImGui::End();
}
void EditorLayer::OnEvent(IEvent& e) {}