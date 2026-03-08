#include "EditorLayer.h"

#include "Application.h"

#include "imgui.h"

DEFINE_LOG_CATEGORY(LogEditorLayer);

EditorLayer::EditorLayer() : ILayer("Editor") {}

void EditorLayer::OnAttach() {
    m_ViewportRenderer = CApplication::Get()->GetRenderEngine()->GetRenderer();
    m_Overlay.UpdateLocation(EOverlayLocation::BottomLeft);

    CZ_LOG(LogEditorLayer, Info, "EditorLayer Attached.");
}

void EditorLayer::OnDetach() {
    auto context = m_ViewportRenderer->GetGraphicContext();
    UFileDialog::Get(context).Shutdown();
    CIconManager::Get(context).Shutdown();
}

void EditorLayer::OnUpdate(FTimeStep ts) {}

void EditorLayer::OnImGuiRender() {
    ImGuiIO& io = ImGui::GetIO();

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
            if (ImGui::MenuItem("Vertical Sync (VSync)", nullptr, &m_VSyncEnabled))
                m_ViewportRenderer->GetWindow()->SetVSync(m_VSyncEnabled);

            if (ImGui::BeginMenu("PowerMode")) {
                EAppPowerMode appPowerMode = CApplication::Get()->GetPowerMode();
                if (ImGui::MenuItem("Performance", nullptr,
                                    appPowerMode == EAppPowerMode::Performance))
                    CApplication::Get()->SetPowerMode(EAppPowerMode::Performance);

                if (ImGui::MenuItem("Balanced", nullptr, appPowerMode == EAppPowerMode::Balanced))
                    CApplication::Get()->SetPowerMode(EAppPowerMode::Balanced);

                if (ImGui::MenuItem("PowerSaving", nullptr,
                                    appPowerMode == EAppPowerMode::PowerSaving))
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
    m_SceneHierarchyPanel.Draw("SceneHierarchy", &m_IsSceneHierarchyOpen);
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
    ImGuiWindowFlags viewportFlags =
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    ImGui::Begin("Viewport##Editor", nullptr, viewportFlags);

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
    m_Overlay.Draw("Editor Overlay:", &m_IsOverlayOpen, [io]() {
        // Performance monitoring
        auto profiler = CApplication::Get()->GetPerformanceProfiler();
        float fps = CApplication::Get()->GetFPSCounter()->GetFPS();
        float latency = CApplication::Get()->GetFPSCounter()->GetAvgLatency();

        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Engine FPS: %.1f", fps);
        ImGui::TextDisabled("Latency: %.3f ms", latency);

        for (uint32_t i = 1; i < (uint32_t)EProfileSlot::COUNT; ++i) {
            const auto& data = profiler->GetSlot((EProfileSlot)i);
            if (data.Samples > 0) {
                ImGui::Text("%-20s: %.3f ms", GProfileSlotNames[i], data.Time);
            }
        }

        // Mouse Position
        // if (ImGui::IsMousePosValid())
        //     ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
        // else
        //     ImGui::Text("Mouse Position: <invalid>");
    });

    ImGui::End();
    ImGui::PopStyleVar();
#pragma endregion

    auto context = m_ViewportRenderer->GetGraphicContext();
    auto& fileDialog = UFileDialog::Get(context);
    if (fileDialog.IsDone("TextureOpenDialog")) {
        if (fileDialog.HasResult()) {
            std::string res = fileDialog.GetResult().string();
            CZ_LOG(LogEditorLayer, Trace, "OPEN[{}]", res);
        }
        fileDialog.Close();
    }
}

void EditorLayer::OnEvent(IEvent& e) {}

void EditorLayer::NewProject() {}

void EditorLayer::OpenProject() {
    auto context = m_ViewportRenderer->GetGraphicContext();

    UFileDialog::Get(context).Open(
        "TextureOpenDialog", "Open a texture",
        "Image file (*.png;*.jpg;*.jpeg;*.bmp;*.tga){.png,.jpg,.jpeg,.bmp,.tga},.*");
}

void EditorLayer::OpenProject(const std::filesystem::path& path) {}

void EditorLayer::SaveProjectAs() {}
