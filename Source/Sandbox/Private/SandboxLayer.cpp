#include "SandboxLayer.h"

#include "Application.h"

#include "imgui.h"

DEFINE_LOG_CATEGORY(LogSandboxLayer);

SandboxLayer::SandboxLayer() : ILayer("Sandbox") {}

void SandboxLayer::OnAttach() {
    m_ViewportRenderer = CApplication::Get()->GetRenderEngine()->GetRenderer();
    m_Viewport = m_ViewportRenderer->CreateViewport("Sandbox", m_ViewportSize.x, m_ViewportSize.y);

    m_Overlay.UpdateLocation(EOverlayLocation::TopRight);

    CZ_LOG(LogSandboxLayer, Info, "SandboxLayer Attached.");
}

void SandboxLayer::OnDetach() {}

void SandboxLayer::OnUpdate(float deltaTime) {}

void SandboxLayer::OnImGuiRender() {
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
            if (ImGui::MenuItem("Vertical Sync (VSync)", nullptr, &m_VSyncEnabled)) {
                m_ViewportRenderer->GetWindow()->SetVSync(m_VSyncEnabled);
            }

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

#pragma region Viewport Rendering
    // ----------------------------------------------------------------------------
    // [Sub-Section] Main Viewport
    // Renders the final scene texture from the Framebuffer.
    // ----------------------------------------------------------------------------
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 }); // Viewport begin
    ImGuiWindowFlags viewportFlags =
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    ImGui::Begin("Viewport##Sandbox", nullptr, viewportFlags);

    // m_ViewportFocused = ImGui::IsWindowFocused();
    // m_ViewportHovered = ImGui::IsWindowHovered();
    // Application::Get().GetImGuiLayer().BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

    auto viewportOffset = ImGui::GetCursorPos(); // includes tab bar
    m_ViewportSize      = ImGui::GetContentRegionAvail();

    // Get DescriptorSet from RHI Texture and draw it as ImGui image
    auto tex              = m_Viewport->GetFrameBuffer()->GetColorAttachment(0);
    ImTextureID textureID = (ImTextureID)tex->GetDescriptorSet();
    ImGui::Image(textureID, m_ViewportSize, ImVec2(0, 1), ImVec2(1, 0));

    // Integrated Debug Overlay
    m_Overlay.Draw("SandboxOverlay:", &m_IsOverlayOpen, [io]() {
        // Performance monitoring
        auto profiler = CApplication::Get()->GetPerformanceProfiler();
        float fps     = CApplication::Get()->GetFPSCounter()->GetFPS();
        float latency = CApplication::Get()->GetFPSCounter()->GetAvgLatency();

        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Engine FPS: %.1f", fps);
        ImGui::TextDisabled("Latency: %.3f ms", latency);

        for (uint32_t i = 1; i < (uint32_t)EProfileSlot::COUNT; ++i) {
            const auto& data = profiler->GetSlot((EProfileSlot)i);
            if (data.Samples > 0) {
                ImGui::Text("%-20s: %.3f ms", GProfileSlotNames[i], data.Time);
            }
        }
    });

    ImGui::End();
    ImGui::PopStyleVar();
#pragma endregion
}

void SandboxLayer::OnEvent(IEvent& e) {}

void SandboxLayer::NewProject() {}

void SandboxLayer::OpenProject() {}

void SandboxLayer::OpenProject(const std::filesystem::path& path) {}

void SandboxLayer::SaveProjectAs() {}
