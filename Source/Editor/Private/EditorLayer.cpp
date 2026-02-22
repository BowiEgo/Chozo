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
    // ----------------------------------------------------------------------------
    // [Section] Dockspace Configuration
    // Set up a full-screen dockspace container for editor panels.
    // ----------------------------------------------------------------------------
    ImGuiWindowFlags dock_space_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    dock_space_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    dock_space_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
    // ----------------------------------------------------------------------------
    ImGui::Begin("DockSpace", nullptr, dock_space_flags);
    ImGui::PopStyleVar();

    // Initialize Docking node if enabled in Config
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();
    float minWinSizeX = style.WindowMinSize.x;
    style.WindowMinSize.x = 300.0f;
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    }
    style.WindowMinSize.x = minWinSizeX;

#pragma region Main Menu Bar
    // ----------------------------------------------------------------------------
    // [Sub-Section] Main Menu Bar
    // ----------------------------------------------------------------------------
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
#pragma endregion

#pragma region Editor Panels
    // ----------------------------------------------------------------------------
    // [Sub-Section] Sub-Panels Update
    // ----------------------------------------------------------------------------
    m_ConsolePanel.Draw("Console", &m_IsConsoleOpen);
    m_SceneHRCPanel.Draw("SceneHierarchy", &m_IsSceneHRCOpen);
    m_PropertiesPanel.Draw("Properties", &m_IsPropertiesOpen);
    m_ContentBrowserPanel.Draw("ContentBrowser", &m_IsContentBrowserOpen);
    m_MaterialPanel.Draw("Material", &m_IsMaterialOpen);
    m_TextureViewerPanel.Draw("TextureViewer", &m_IsTextureViewerOpen);
    m_AssetsPanel.Draw("Assets", &m_IsAssetsOpen);
#pragma endregion

#pragma region Viewport Rendering
    // ----------------------------------------------------------------------------
    // [Sub-Section] Main Viewport
    // Renders the final scene texture from the Framebuffer.
    // ----------------------------------------------------------------------------
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 }); // Viewport begin
    ImGui::Begin("Viewport");

    // m_ViewportFocused = ImGui::IsWindowFocused();
    // m_ViewportHovered = ImGui::IsWindowHovered();
    // Application::Get().GetImGuiLayer().BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

    auto viewportOffset = ImGui::GetCursorPos(); // includes tab bar
    m_ViewportSize = ImGui::GetContentRegionAvail();

    // Get DescriptorSet from RHI Texture and draw it as ImGui image
    auto texture = m_ViewportRenderer->GetSceneFrameBuffer()->GetColorAttachment(0);
    ImTextureID textureID = (ImTextureID)texture->GetDescriptorSet();
    ImGui::Image(textureID, m_ViewportSize, ImVec2(0, 1), ImVec2(1, 0));

    // Integrated Debug Overlay
    m_Overlay.Draw(&m_IsOverlayOpen, [io]() {
        // Performance monitoring
        auto profiler = CApplication::Get()->GetPerformanceProfiler();
        auto& profilerData = profiler->GetPerFrameData();

        if (auto* totalData = profiler->GetEntry(ProfilerKeys::TotalFrame)) {
            float totalTime = totalData->Time;
            float fps = (totalTime > 0.0f) ? (1000.0f / totalTime) : 0.0f;

            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Engine FPS: %.1f", fps);
            ImGui::TextDisabled("Latency: %.3f ms", totalTime);
            ImGui::Separator();
        }

        for (const auto& [name, data] : profilerData) {
            if (name == ProfilerKeys::TotalFrame) continue;

            float avgTime = data.Time / (data.Samples > 0 ? data.Samples : 1);
            ImGui::Text("%-20s: %.3f ms", name.c_str(), avgTime);
        }

        profiler->Clear();

        // Mouse Position
        if (ImGui::IsMousePosValid())
            ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
        else
            ImGui::Text("Mouse Position: <invalid>");
    });

    ImGui::End();
    ImGui::PopStyleVar();
#pragma endregion

    ImGui::End(); // End Dockspace
}

void EditorLayer::OnEvent(IEvent& e) {}

void EditorLayer::NewProject() {}

void EditorLayer::OpenProject() {}

void EditorLayer::OpenProject(const std::filesystem::path& path) {}

void EditorLayer::SaveProjectAs() {}
