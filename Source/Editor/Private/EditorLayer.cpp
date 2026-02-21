#include "EditorLayer.h"

#include "Application.h"

#include "imgui.h"

EditorLayer::EditorLayer() : ILayer("Editor") {}

void EditorLayer::OnAttach() {
    m_ViewportRenderer = CApplication::Get()->GetRenderEngine()->GetRenderer();
    m_Overlay.UpdateLocation(EOverlayLocation::BottomLeft);
}

void EditorLayer::OnDetach() {}

void EditorLayer::OnUpdate(FTimeStep ts) {}

void EditorLayer::OnImGuiRender() {
    ImGuiWindowFlags dock_space_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    dock_space_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    dock_space_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
    //-------------------- Dockspace begin -------------------------------------
    ImGui::Begin("DockSpace", nullptr, dock_space_flags);
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
    // Menu
    // --------------------
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) NewProject();
            if (ImGui::MenuItem("Open...", "Ctrl+O")) OpenProject();
            if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S", nullptr)) SaveProjectAs();
            if (ImGui::MenuItem("Quit")) CApplication::Get()->Exit();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Renderer")) {
            if (ImGui::MenuItem("Recompile Shaders")) {
                // Renderer::GetShaderLibrary()->Recompile();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Settings")) {
            if (ImGui::MenuItem("Vertical Sync (VSync)", nullptr, &m_VSyncEnabled)) {
                EPresentMode Mode =
                    m_VSyncEnabled ? EPresentMode::FIFO_Relaxed : EPresentMode::Immediate;

                m_ViewportRenderer->GetGraphicsContext()->GetRHI()->GetSwapchain()->SetPresentMode(
                    Mode);
            }

            if (ImGui::BeginMenu("PowerMode")) {
                if (ImGui::MenuItem("Performance"))
                    CApplication::Get()->SetPowerMode(EAppPowerMode::Performance);

                if (ImGui::MenuItem("Balanced"))
                    CApplication::Get()->SetPowerMode(EAppPowerMode::Balanced);

                if (ImGui::MenuItem("PowerSaving"))
                    CApplication::Get()->SetPowerMode(EAppPowerMode::PowerSaving);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    // ----------------------------------------------------------------------
    // Viewport
    // ----------------------------------------------------------------------
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

    // Overlay
    m_Overlay.OnImGuiRender(&m_IsOverlayOpen, [io]() {
        // auto& profilerData = CApplication::Get()->GetPerformanceProfiler()->GetPerFrameData();
        // for (const auto& [name, data] : profilerData) {
        //     // Calculate average if multiple samples were taken in one frame
        //     float avgTime = data.Time / (data.Samples > 0 ? data.Samples : 1);
        //     ImGui::Text("%s: %.3f ms", name, avgTime);
        // }
        // CApplication::Get()->GetPerformanceProfiler()->Clear();

        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        if (ImGui::IsMousePosValid())
            ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
        else
            ImGui::Text("Mouse Position: <invalid>");
    });

    ImGui::End();
    ImGui::PopStyleVar();

    //-------------------- Dockspace end -------------------------------------
    ImGui::End();
}

void EditorLayer::OnEvent(IEvent& e) {}

void EditorLayer::NewProject() {}

void EditorLayer::OpenProject() {}

void EditorLayer::OpenProject(const std::filesystem::path& path) {}

void EditorLayer::SaveProjectAs() {}
