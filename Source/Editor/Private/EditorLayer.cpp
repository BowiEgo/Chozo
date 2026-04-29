#include "EditorLayer.h"

#include "Application.h"
#include "AssetManager.h"
#include "EditorEvent.h"
#include "ImGuiLayer.h"
#include "Input.h"
#include "Material.h"
#include "PBRMaterialParams.h"
#include "UIUtils.h"

#include "SphereParams.h"

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
    // auto& registry = FMeshRegister::Get();
    // for (const auto& typeName : registry.GetMeshTypeNames()) {
    //     CZ_LOG(LogEditorLayer, Info, "Available mesh type: {}", typeName);
    // }

    auto fbSize = CApplication::Get()->GetWindow()->GetFrameBufferSize();

    m_Scene            = CreateScope<FScene>();
    m_ViewportRenderer = CApplication::Get()->GetRenderEngine()->GetRenderer();
    m_Viewport = m_ViewportRenderer->CreateViewport("Editor", m_ViewportSize.x, m_ViewportSize.y);
    m_Viewport->SetScene(m_Scene.get());

    auto context = m_ViewportRenderer->GetGraphicContext();
    m_ContentBrowserPanel.SetContext(context);

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
                m_SyncLayer->UnregisterNode(event.GetNode());
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

    {
        // Material

        auto nodeBit   = FTypeRegister::Get().GetBit("Node_Regular");
        auto sphereBit = FTypeRegister::Get().GetBit("Mesh_Sphere");
        auto newNode   = m_NodeTree.CreateNode("Sphere", nodeBit |= sphereBit, nullptr);
        m_NodeTree.SelectNode(newNode);

        auto pbrMat = m_ViewportRenderer->GetMaterial();
        static_cast<FSphereParams*>(newNode->GetMeshParamsWrapper()->Get())->Material =
            pbrMat->GetHandle();

        auto meshProps = newNode->GetMeshParamsWrapper();
    }

    {
        auto nodeBit = FTypeRegister::Get().GetBit("Node_Regular");
        auto hdriBit = FTypeRegister::Get().GetBit("Light_HDRIBackdrop");
        auto newNode = m_NodeTree.CreateNode("HDRI Backdrop", nodeBit |= hdriBit, nullptr);

        auto tex = CAssetManager::Get().GetOrLoadTexture("textures://HDRI/newport_loft.hdr");
        newNode->GetHDRIBackdropParams()->Cubemap = tex->GetHandle();
    }

    FEventBus::Get().AddListener(EEventType::OpenMaterialPanel, [this](IEvent& e) {
        auto& openMaterialPanelEvent = static_cast<FOpenMaterialPanelEvent&>(e);

        CZ_LOG(LogEditorLayer, Trace, "OpenMaterialPanel: {}",
               openMaterialPanelEvent.GetMaterialHandle().ToString());

        auto mat = CAssetManager::Get().GetAsset(openMaterialPanelEvent.GetMaterialHandle());
        if (mat) {
            m_MaterialPanel.SetMaterial(mat);
            m_MaterialPanel.Open();
            ImGuiWindow* window = ImGui::FindWindowByName("Material");
            if (window) {
                ImGui::FocusWindow(window);
            }
        }
        return true; // Return false to indicate we don't want to mark the event as handled
    });

    m_ConsolePanel.Open();
    m_SceneHierarchyPanel.Open();
    m_PropertiesPanel.Open();
    m_ContentBrowserPanel.Open();
    m_MaterialPanel.Open();
    m_TextureViewerPanel.Open();
    m_AssetsPanel.Open();
}

void EditorLayer::OnDetach() {
    // m_NodeTree->UnregisterEventCallback(handle);
}

void EditorLayer::OnUpdate(float deltaTime) {
    m_Viewport->Resize(m_ViewportSize.x, m_ViewportSize.y);
    m_EditorCamera.OnUpdate(deltaTime, m_ViewportFocused);
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
                if (ImGui::MenuItem("NoLimit", nullptr, appPowerMode == EAppPowerMode::NoLimit))
                    CApplication::Get()->SetPowerMode(EAppPowerMode::NoLimit);
                if (ImGui::MenuItem("Extreme", nullptr, appPowerMode == EAppPowerMode::Extreme))
                    CApplication::Get()->SetPowerMode(EAppPowerMode::Extreme);
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

        if (ImGui::BeginMenu("Debug")) {
            static int item_selected_idx = 0;
            int currentDebugMode =
                CApplication::Get()->GetRenderEngine()->GetRenderer()->GetDebugMode();
            if (ImGui::MenuItem("Position", nullptr, currentDebugMode == 0)) item_selected_idx = 0;
            if (ImGui::MenuItem("Normal", nullptr, currentDebugMode == 1)) item_selected_idx = 1;
            if (ImGui::MenuItem("BaseColor", nullptr, currentDebugMode == 2)) item_selected_idx = 2;
            if (ImGui::MenuItem("Roughness", nullptr, currentDebugMode == 3)) item_selected_idx = 3;
            if (ImGui::MenuItem("Metallic", nullptr, currentDebugMode == 4)) item_selected_idx = 4;
            if (ImGui::MenuItem("AO", nullptr, currentDebugMode == 5)) item_selected_idx = 5;
            if (ImGui::MenuItem("Emissive", nullptr, currentDebugMode == 6)) item_selected_idx = 6;
            if (ImGui::MenuItem("Depth", nullptr, currentDebugMode == 7)) item_selected_idx = 7;

            CApplication::Get()->GetRenderEngine()->GetRenderer()->SetDebugMode(item_selected_idx);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
#pragma endregion

#pragma region Editor Panels
    // ----------------------------------------------------------------------------
    // [Sub-Section] Sub-Panels Update
    // ----------------------------------------------------------------------------
    m_ConsolePanel.Draw("Console");
    m_SceneHierarchyPanel.Draw("Scene Hierarchy");
    m_PropertiesPanel.Draw("Properties");
    m_ContentBrowserPanel.Draw("Content Browser");
    m_MaterialPanel.Draw("Material");
    m_TextureViewerPanel.Draw("Texture Viewer");
    m_AssetsPanel.Draw("Assets");
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

    // Block event pass-through to the underlying scene when the viewport window is neither focused
    // nor hovered.
    // This ensures that ImGui handles input when interacting with other UI elements, while allowing
    // the scene to receive input when the viewport is active.
    m_ViewportFocused = ImGui::IsWindowFocused();
    m_ViewportHovered = ImGui::IsWindowHovered();
    CImGuiLayer::Get().BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

    auto viewportOffset = ImGui::GetCursorPos(); // includes tab bar
    m_ViewportSize      = ImGui::GetContentRegionAvail();

    // Get DescriptorSet from RHI Texture and draw it as ImGui image
    auto tex              = m_Viewport->GetFrameBuffer()->GetColorAttachment(0);
    ImTextureID textureID = GET_IM_RHI_TEXTURE_ID(tex.get());
    ImGui::Image(textureID, m_ViewportSize, ImVec2(1, 0), ImVec2(0, 1));

    // Integrated Debug Overlay
    m_Overlay.Draw("Editor Overlay:", &m_IsOverlayOpen, [io]() {
        // Performance monitoring
        auto appProfiler = CApplication::Get()->GetPerformanceProfiler();
        float fps        = CApplication::Get()->GetFPSCounter()->GetFPS();
        float latency    = CApplication::Get()->GetFPSCounter()->GetAvgLatency();

        auto rendererProfiler =
            CApplication::Get()->GetRenderEngine()->GetRenderer()->GetPerformanceProfiler();

        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Engine FPS: %.1f", fps);
        ImGui::TextDisabled("Latency: %.3f ms", latency);
        for (uint32_t i = 1; i < (uint32_t)EAppProfileSlot::COUNT; ++i) {
            const float time = appProfiler->GetSmoothedAverage((uint32_t)(EAppProfileSlot)i);
            ImGui::Text("%-20s: %.3f ms", GAppProfileSlotNames[i], time);
        }

        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Renderer:");
        for (uint32_t i = 1; i < (uint32_t)ERendererProfileSlot::COUNT; ++i) {
            const float time =
                rendererProfiler->GetSmoothedAverage((uint32_t)(ERendererProfileSlot)i);
            ImGui::Text("%-20s: %.3f ms", GRendererProfileSlotNames[i], time);
        }

        auto gpuProfiler = CApplication::Get()->GetGPUProfiler();
        float mbSize     = 1024.0 * 1024.0;

        for (auto& heapInfo : gpuProfiler.Heaps) {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "GPU: %s", heapInfo.Type.c_str());
            ImGui::Text("Size: %.3f MB", heapInfo.Size / mbSize);
            ImGui::Text("Budget: %.3f MB", heapInfo.Budget / mbSize);
            ImGui::Text("Usage: %.3f MB", heapInfo.Usage / mbSize);
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

    auto context     = m_ViewportRenderer->GetGraphicContext();
    auto& fileDialog = UFileDialog::Get();
    if (fileDialog.IsDone("TextureOpenDialog")) {
        if (fileDialog.HasResult()) {
            std::string res = fileDialog.GetResult().string();
            CZ_LOG(LogEditorLayer, Trace, "OPEN[{}]", res);
        }
        fileDialog.Close();
    }
}

void EditorLayer::OnEvent(IEvent& e) {
    if (m_ViewportFocused && m_ViewportHovered) m_EditorCamera.OnEvent(e);

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
        case Key::F: {
            CZ_LOG(LogEditorLayer, Trace, "F Pressed");
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

    UFileDialog::Get().Open(
        "TextureOpenDialog", "Open a texture",
        "Image file (*.png;*.jpg;*.jpeg;*.bmp;*.tga){.png,.jpg,.jpeg,.bmp,.tga},.*");
}

void EditorLayer::OpenProject(const std::filesystem::path& path) {}

void EditorLayer::SaveProjectAs() {}
