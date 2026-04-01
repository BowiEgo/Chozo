#include "EditorLayer.h"

#include "Application.h"
#include "Input.h"

#include "imgui.h"

DEFINE_LOG_CATEGORY(LogEditorLayer);

EditorLayer::EditorLayer() : ILayer("Editor") {}

void EditorLayer::OnAttach() {
    // FPluginManager::Get().Initialize("bin/plugins");
    // // FPluginManager::Get().LoadPlugin("Cube");

    // for (const auto& pluginName : pluginMgr.GetAvailablePlugins()) {
    //     pluginMgr.LoadPlugin(pluginName);
    // }

    // // Get all available mesh types
    // auto& registry = FMeshRegistry::Get();
    // for (const auto& typeName : registry.GetMeshTypeNames()) {
    //     CZ_LOG(LogEditorLayer, Info, "Available mesh type: {}", typeName);
    // }

    auto fbSize = CApplication::Get()->GetWindow()->GetFrameBufferSize();

    m_Scene = CreateScope<FScene>();
    m_ViewportRenderer = CApplication::Get()->GetRenderEngine()->GetRenderer();
    m_Viewport = m_ViewportRenderer->CreateViewport("Editor", m_ViewportSize.x, m_ViewportSize.y);
    m_Viewport->SetScene(m_Scene.get());

    m_Overlay.UpdateLocation(EOverlayLocation::BottomLeft);

    auto mainCamera = m_Viewport->GetCamera();
    m_EditorCamera.SetActiveCamera(mainCamera);
    mainCamera->SetPerspective(45.0f, (float)fbSize.Width / fbSize.Height, 0.1f, 1000.0f);
    mainCamera->SetPosition(FVector3(0, 0, 5));

    m_SyncLayer = CreateScope<FSyncLayer>(m_Scene.get());

    CallbackHandle handle = m_NodeTree.RegisterEventCallback([this](const FNodeEvent& event) {
        switch (event.GetType()) {
            case ENodeEventType::Created:
                m_SyncLayer->RegisterNode(event.GetNode());
                CZ_LOG(LogEditorLayer, Trace, "Node created: {}", event.GetNode()->GetName());
                break;
            case ENodeEventType::Deleted:
                CZ_LOG(LogEditorLayer, Trace, "Node deleted: {}", event.GetNode()->GetName());
                break;
            case ENodeEventType::Renamed:
                CZ_LOG(LogEditorLayer, Trace, "Node renamed: {} -> {}", event.GetOldName(),
                       event.GetNode()->GetName());
                break;
            case ENodeEventType::Moved:
                CZ_LOG(LogEditorLayer, Trace, "Node parent changed: {}",
                       event.GetNode()->GetName());
                break;
            case ENodeEventType::Selected:
                CZ_LOG(LogEditorLayer, Trace, "Node selected: {}", event.GetNode()->GetName());
                break;
            case ENodeEventType::DirtyChanged:
                CZ_LOG(LogEditorLayer, Trace, "Node dirty changed: {}", event.GetNode()->GetName());
                break;
            default:
                CZ_LOG(LogEditorLayer, Warning, "Unknown event type: {}",
                       static_cast<int>(event.GetType()));
                break;
        }
    });

    m_SceneHierarchyPanel.SetNodeTree(&m_NodeTree);
    m_PropertiesPanel.SetNodeTree(&m_NodeTree);

    CZ_LOG(LogEditorLayer, Info, "EditorLayer Attached.");
}

void EditorLayer::OnDetach() {
    auto context = m_ViewportRenderer->GetGraphicContext();
    UFileDialog::Get(context).Shutdown();
    CIconManager::Get(context).Shutdown();

    // m_NodeTree->UnregisterEventCallback(handle);
}

void EditorLayer::OnUpdate(float deltaTime) {
    m_Viewport->Resize(m_ViewportSize.x, m_ViewportSize.y);
    m_EditorCamera.OnUpdate(deltaTime);
    // m_EditorCamera.CopyTo(m_Viewport->GetCamera());

    m_SyncLayer->SyncAllNodesToEntities();
}

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
    m_SceneHierarchyPanel.Draw("Scene Hierarchy", &m_IsSceneHierarchyOpen);
    m_PropertiesPanel.Draw("Properties", &m_IsPropertiesOpen);
    m_ContentBrowserPanel.Draw("Content Browser", &m_IsContentBrowserOpen);
    m_MaterialPanel.Draw("Material", &m_IsMaterialOpen);
    m_TextureViewerPanel.Draw("Texture Viewer", &m_IsTextureViewerOpen);
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
    ImTextureID textureID = (ImTextureID)m_Viewport->GetTextureID(0);
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

void EditorLayer::OnEvent(IEvent& e) {
    m_EditorCamera.OnEvent(e);

    FEventDispatcher dispatcher(e);
    dispatcher.Dispatch<FKeyPressedEvent>(CZ_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
    // dispatcher.Dispatch<FMouseButtonPressedEvent>(
    //     CZ_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
    // dispatcher.Dispatch<FMouseButtonReleasedEvent>(
    //     CZ_BIND_EVENT_FN(EditorLayer::OnMouseButtonReleased));
}

bool EditorLayer::OnKeyPressed(FKeyPressedEvent& e) {
    // Shortcuts
    if (e.GetRepeatCount() > 0) return false;

    bool control =
        SInput::IsKeyPressed(Key::LeftControl) || SInput::IsKeyPressed(Key::RightControl);
    bool shift = SInput::IsKeyPressed(Key::LeftShift) || SInput::IsKeyPressed(Key::RightShift);
    switch (e.GetKeyCode()) {
        case Key::N: {
            if (control) NewProject();
            break;
        }
        case Key::O: {
            if (control) OpenProject();
            break;
        }
        case Key::S: {
            if (control && shift) SaveProjectAs();
            break;
        }

        // Gizmos
        // case Key::Q: m_GizmoType = -1; break;
        // case Key::W: m_GizmoType = ImGuizmo::OPERATION::TRANSLATE; break;
        // case Key::E: m_GizmoType = ImGuizmo::OPERATION::ROTATE; break;
        // case Key::R: m_GizmoType = ImGuizmo::OPERATION::SCALE; break;
        case Key::F9: {
            CZ_LOG(LogEditorLayer, Trace, "F9 Pressed");
            if (m_PolygonMode == EPolygonMode::Fill) {
                m_ViewportRenderer->SetPolygonMode(EPolygonMode::Line);
                m_PolygonMode = EPolygonMode::Line;
            } else {
                m_ViewportRenderer->SetPolygonMode(EPolygonMode::Fill);
                m_PolygonMode = EPolygonMode::Fill;
            }
        }
        default: break;
    }
    return true;
}

// bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e) { return false; }

// bool EditorLayer::OnMouseButtonReleased(MouseButtonReleasedEvent& e) { return false; }

void EditorLayer::NewProject() {}

void EditorLayer::OpenProject() {
    auto context = m_ViewportRenderer->GetGraphicContext();

    UFileDialog::Get(context).Open(
        "TextureOpenDialog", "Open a texture",
        "Image file (*.png;*.jpg;*.jpeg;*.bmp;*.tga){.png,.jpg,.jpeg,.bmp,.tga},.*");
}

void EditorLayer::OpenProject(const std::filesystem::path& path) {}

void EditorLayer::SaveProjectAs() {}
