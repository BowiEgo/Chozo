#include <Core/Event/Input.hpp>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <string>
#include <string_view>

#include "EditorLayer.hpp"

#include "Core/Memory/Memory.hpp"
#include "UIUtils.hpp"

#include <Runtime/App/Application.hpp>
#include <Runtime/RHI/RHIAPI.hpp>

using namespace CZ;

EditorLayer::EditorLayer() {}

EditorLayer::~EditorLayer() {}

void EditorLayer::OnAttach() {
    auto window = Application::Get().GetWindow();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport

#ifdef CZ_PLATFORM_WINDOWS
    ImFontConfig config;
    config.MergeMode  = true;
    config.PixelSnapH = true;

    const char* chineseFontPath = "C:\\Windows\\Fonts\\msyh.ttc";
    io.Fonts->AddFontFromFileTTF(chineseFontPath, 18.0f * 1.5f, &config,
                                 io.Fonts->GetGlyphRangesChineseFull());
#endif
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look
    // identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding              = 0.0f;
        style.FrameRounding               = 2.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    float pixelRatio = window->GetPixelRatio();
    Extent2D fbScale = window->GetFrameBufferScale();
    style.ScaleAllSizes(fbScale.Width / pixelRatio);

    SetDarkThemeColors();

    auto fbSize = window->GetFrameBufferSize();

    {
        std::string uiError;
        m_ImGuiRenderer = CreateUIRenderBackend(uiError);

        if (m_ImGuiRenderer &&
            !m_ImGuiRenderer->Init(window, Application::Get().GetEngine()->GetGraphicContext(),
                                   uiError)) {
            CZ_EDITOR_LOG(Error, "Failed to initialise the UI backend: {}", uiError);
            m_ImGuiRenderer.reset();
        }

        if (!m_ImGuiRenderer) {
            CZ_EDITOR_LOG(Error, "The editor runs without a UI backend: {}", uiError);
        }
    }

    m_ViewportRenderer = Application::Get().GetEngine()->GetRenderer();
    m_Viewport = m_ViewportRenderer.CreateViewport("Editor", m_ViewportSize.x, m_ViewportSize.y);

    // The viewport owns the scene; the editor only holds a view of it.
    m_Scene      = m_Viewport->GetScene();
    m_SyncBridge = CZ_CREATE_SCOPE(MEMORY_USAGE_UI, SyncBridge, m_Scene);

    auto mainCamera = m_Viewport->GetCamera();
    m_EditorCamera.SetActiveCamera(mainCamera);
    mainCamera->SetPerspective(45.0f, (float)fbSize.Width / fbSize.Height, 0.1f, 1000.0f);
    // The orbit camera scales panning by the focal distance and by the viewport size (see
    // EditorCamera::MousePan / PanSpeed). Leaving them at zero makes pan and zoom silent no-ops,
    // so seed them before the explicit position below has the final word.
    mainCamera->SetDistance(5.0f);
    mainCamera->SetViewportSize((float)fbSize.Width, (float)fbSize.Height);
    mainCamera->SetPosition(Vector3(0, 0, 5));

    CallbackHandle handle = m_NodeTree.RegisterEventCallback([this](const NodeEvent& event) {
        switch (event.GetType()) {
            case EditorNodeEventType::Created: {
                EditorNode* created = event.GetNode();
                if (created && created->HasMesh()) {
                    if (MeshParamsObj* raw = created->GetMeshParams().As<MeshParamsObj>()) {
                        m_DefaultSnapshots.emplace(raw->GetTypeName(), CaptureParams(*raw));
                    }
                }
                m_SyncBridge->RegisterNode(created);
                break;
            }
            case EditorNodeEventType::Deleted: m_SyncBridge->UnregisterNode(event.GetNode()); break;
            case EditorNodeEventType::Renamed: break;
            case EditorNodeEventType::Moved: break;
            case EditorNodeEventType::Selected: break;
            case EditorNodeEventType::DirtyChanged: break;
            default: break;
        }
    });

    m_NodeTree.Init();

    m_SceneHierarchyPanel.SetNodeTree(&m_NodeTree);
    m_PropertiesPanel.SetNodeTree(&m_NodeTree);
    m_PropertiesPanel.SetDefaults(&m_DefaultSnapshots);
    m_PropertiesPanel.SetCommandStack(&m_Commands);

    // Shortcuts are declared once: chord, name (shown in Help -> Shortcuts) and action.
    m_Shortcuts.Add(KeyChord::Primary(CZ_KEY(Z)), "Undo", [this]() { m_Commands.Undo(); });
    m_Shortcuts.Add(KeyChord::Primary(CZ_KEY(Z)).WithShift(), "Redo",
                    [this]() { m_Commands.Redo(); });
    m_Shortcuts.Add(KeyChord::Of(CZ_KEY(F5)), "Toggle wireframe", [this]() {
        m_ViewportRenderer.SetWireframe(!m_ViewportRenderer.IsWireframe());
    });
    m_Shortcuts.Add(KeyChord::Of(CZ_KEY(F9)), "Toggle performance overlay",
                    [this]() { m_PerfOverlay.Toggle(); });

    m_ConsolePanel.Open();
    m_SceneHierarchyPanel.Open();
    m_PropertiesPanel.Open();

    {
        // Material

        auto cubeNodeMask   = GET_NODE_MASK("Node_Regular", "ProceduralMesh_Cube");
        EditorNode* newNode = m_NodeTree.CreateNode("Cube", cubeNodeMask, nullptr);
        m_NodeTree.SelectNode(newNode);

        // Node defaults place the transform at (1, 1, 1); the camera looks at the world origin, so
        // move the default cube there to have it centred in the viewport.
        newNode->SetTransformParams(
            CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, TransformParamsObj, Vector3(0.0f, 0.0f, 0.0f)));
    }

    {
        auto sphereNodeMask = GET_NODE_MASK("Node_Regular", "ProceduralMesh_Sphere");
        EditorNode* sphere  = m_NodeTree.CreateNode("Sphere", sphereNodeMask, nullptr);
        sphere->SetTransformParams(
            CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, TransformParamsObj, Vector3(2.0f, 0.0f, 0.0f)));

        // One of every other primitive, placed behind the first row so the default view shows all
        // five mesh generators at once (delete the ones you do not want).
        const char* names[3]   = { "Plane", "Cylinder", "Torus" };
        const char* types[3]   = { "ProceduralMesh_Plane", "ProceduralMesh_Cylinder",
                                   "ProceduralMesh_Torus" };
        const Vector3 spots[3] = { Vector3(-1.5f, 0.0f, 0.0f), Vector3(-1.0f, 0.0f, -1.5f),
                                   Vector3(1.5f, 0.0f, -2.5f) };
        for (int i = 0; i < 3; ++i) {
            EditorNode* extra =
                m_NodeTree.CreateNode(names[i], GET_NODE_MASK("Node_Regular", types[i]), nullptr);
            extra->SetTransformParams(
                CZ_CREATE_SCOPE(MEMORY_USAGE_RENDER, TransformParamsObj, spots[i]));
        }

        // static_cast<FSphereParams*>(newNode->GetMeshParamsWrapper()->Get())->Material =
    }
}

void EditorLayer::OnDetach() {
    if (m_ImGuiRenderer) {
        m_ImGuiRenderer->ReleaseAllTextures();
        m_ImGuiRenderer->Shutdown();
        m_ImGuiRenderer.reset();
    }

    ImGui::DestroyContext();
}

void EditorLayer::OnUpdate(float deltaTime) {
    // The registry owns edge detection; the editor supplies the key state and the rule that a
    // focused text field keeps the keyboard, so typing never triggers an editor shortcut.
    for (std::string_view shortcut :
         m_Shortcuts.Update([](KeyCode key) { return Input::IsKeyPressed(key); },
                            []() {
                                // Typing wins, and so does an in-progress drag: ImGui re-applies
                                // the accumulated mouse delta on every frame of a DragFloat, so
                                // undoing mid-drag writes the value straight back and produces a
                                // number nobody asked for.
                                return ImGui::GetIO().WantTextInput || ImGui::IsAnyItemActive();
                            })) {
        CZ_EDITOR_LOG(Warning, "Shortcut: {}", shortcut);
    }

    // Undo/redo through the engine's own input, the same way the viewport shortcuts work. The
    // engine delivers ASCII codes for letters (Z is 90) and SDL range values for the modifiers, and
    // its polling reports both, so the chord is assembled from polls and edge-triggered here rather
    // than relying on any backend's key mapping.
    const auto updateBegin = std::chrono::steady_clock::now();
    m_Viewport->Resize(m_ViewportSize.x, m_ViewportSize.y);
    m_Viewport->GetCamera()->SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
    // Hovering is enough to drive the camera (no click-to-focus required); the values come from
    // the previous frame's ImGui pass, which is the standard one-frame-late pattern.
    // A drag that started inside the viewport keeps steering even if the cursor leaves the panel
    // mid-gesture; otherwise only hover drives the camera (no focus required).
    const bool bDragging =
        Input::IsKeyPressed(CZ_KEY(LeftAlt)) && (Input::IsMouseButtonPressed(MouseButton::Left) ||
                                                 Input::IsMouseButtonPressed(MouseButton::Middle) ||
                                                 Input::IsMouseButtonPressed(MouseButton::Right));
    if (m_ViewportHovered && bDragging) m_CameraDragActive = true;
    if (!bDragging) m_CameraDragActive = false;

    m_EditorCamera.OnUpdate(deltaTime, m_ViewportHovered || m_CameraDragActive);
    // The editor camera is a separate object: without pushing its state onto the camera the
    // renderer actually renders with, any camera input stays invisible.
    m_EditorCamera.CopyTo(m_Viewport->GetCamera());
    m_SyncBridge->SyncAllNodesToEntities();

    const double updateSeconds =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - updateBegin).count();
    const PhaseSample phases[2] = { { "Layer Update", static_cast<float>(updateSeconds) },
                                    { "Frame - Update", static_cast<float>(deltaTime) -
                                                            static_cast<float>(updateSeconds) } };
    m_PerfOverlay.PushFrame(deltaTime, phases, 2);

    const auto gpuTiming           = m_ViewportRenderer.GetGpuTiming();
    const PhaseSample gpuPhases[1] = { { "GPU Frame",
                                         gpuTiming.bValid ? gpuTiming.FrameSeconds : 0.0f } };
    m_PerfOverlay.Stats().SetGpuPhases(gpuPhases, gpuTiming.bValid ? 1 : 0);

    const DrawStats& draws = m_ViewportRenderer.GetDrawStats();
    m_PerfOverlay.Stats().SetDrawCalls(draws.DrawCalls, draws.Triangles);
}

void EditorLayer::OnRender() {
    m_ImGuiRenderer->NewFrame();

    // Viewport rectangle in screen space; the performance overlay anchors to it after the
    // dockspace has closed.
    ImVec2 viewportRectMin{ 0.0f, 0.0f };

    // ----------------------------------------------------------------------------
    // [Section] Dockspace Configuration
    // Set up a full-screen dockspace container for editor panels.
    // ----------------------------------------------------------------------------
    ImGuiWindowFlags dock_space_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    ImGuiViewport* viewport           = ImGui::GetMainViewport();
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
    ImGuiIO& io           = ImGui::GetIO();
    ImGuiStyle& style     = ImGui::GetStyle();
    float minWinSizeX     = style.WindowMinSize.x;
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

        ImGui::BeginDisabled(!m_Commands.CanUndo());
        if (ImGui::Button("Undo")) m_Commands.Undo();
        ImGui::EndDisabled();
        if (ImGui::IsItemHovered() && m_Commands.CanUndo()) {
            ImGui::SetTooltip("Undo %s", std::string(m_Commands.UndoLabel()).c_str());
        }
        ImGui::SameLine();
        ImGui::BeginDisabled(!m_Commands.CanRedo());
        if (ImGui::Button("Redo")) m_Commands.Redo();
        ImGui::EndDisabled();
        if (ImGui::IsItemHovered() && m_Commands.CanRedo()) {
            ImGui::SetTooltip("Redo %s", std::string(m_Commands.RedoLabel()).c_str());
        }
        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) NewProject();
            if (ImGui::MenuItem("Open...", "Ctrl+O")) OpenProject();
            if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S", nullptr)) SaveProjectAs();
            if (ImGui::MenuItem("Quit")) Application::Get().Close();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Renderer")) {
            if (ImGui::MenuItem("Recompile Shaders")) {
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            ImGui::MenuItem("Shortcuts", nullptr, &m_bShowShortcuts);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Settings")) {

            //     if (ImGui::MenuItem("Balanced", nullptr, appPowerMode ==

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
    m_BlockEvents     = !m_ViewportFocused && !m_ViewportHovered;

    auto viewportOffset = ImGui::GetCursorPos(); // includes tab bar
    m_ViewportSize      = ImGui::GetContentRegionAvail();

    // Get DescriptorSet from RHI Texture and draw it as ImGui image. The viewport framebuffer is
    // recreated on resize, so the previously registered texture (and its ImGui descriptor set,
    // which references the old image view) has to be released first.
    auto tex = m_Viewport->GetFrameBuffer()->GetColorAttachment(0);

    if (tex && tex->GetID() != m_ViewportTextureID) {
        if (m_ViewportTextureID.IsValid()) {
            m_ImGuiRenderer->ReleaseTexture(m_ViewportTextureID);
        }

        m_ViewportTextureID = tex->GetID();
    }

    ImTextureID textureID = GET_IM_TEXTURE_ID(tex);
    // Top-left of the render target in screen space; the performance overlay anchors to it.
    viewportRectMin       = ImGui::GetCursorScreenPos();
    ImGui::Image(textureID, m_ViewportSize, ImVec2(1, 0), ImVec2(0, 1));

    // // Integrated Debug Overlay
    //     // Performance monitoring

    //     auto rendererProfiler =

    //         const float time =

    //     // Mouse Position
    //     // if (ImGui::IsMousePosValid())
    //     //     ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
    //     // else
    //     //     ImGui::Text("Mouse Position: <invalid>");

    ImGui::End();
    ImGui::PopStyleVar();
#pragma endregion

    ImGui::End(); // End Dockspace

    // Drawn after every window has closed, as a top-level overlay: inside the dockspace host (or a
    // panel) ImGui treats it as a child of that window and clips it, which is why it stayed
    // invisible no matter where in the panel it was placed.
    // Help -> Shortcuts: generated from the registry, so it cannot go stale.
    if (m_bShowShortcuts) {
        if (ImGui::Begin("Shortcuts", &m_bShowShortcuts)) {
            if (ImGui::BeginTable("##shortcuts", 2, ImGuiTableFlags_SizingStretchProp)) {
                for (const ShortcutRegistry::Entry& entry : m_Shortcuts.Entries()) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(entry.Name.c_str());
                    ImGui::TableNextColumn();
                    ImGui::TextDisabled("%s", ToString(entry.Chord).c_str());
                }
                ImGui::EndTable();
            }
        }
        ImGui::End();
    }

    m_PerfOverlay.Draw(m_PerfPainter, viewportRectMin.x + 8.0f, viewportRectMin.y + 8.0f,
                       std::max(160.0f, m_ViewportSize.x - 24.0f));

    ImGui::Render();

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void EditorLayer::OnEvent(Event& e) {
    // Same wiring as the reference branch: the layer dispatches the events it cares about to its
    // own handlers, because Application only forwards OnEvent and never the key/mouse hooks.
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<KeyPressedEvent>(CZ_BIND_FN(EditorLayer::OnKeyPressed));

    // The editor camera sits outside the ImGui layer, so it needs the raw events (mouse scrolling
    // in particular) forwarded before the capture check below marks them handled.
    // Only steer the camera while the cursor is over the viewport. The reference branch requires
    // focus *and* hover here; hover alone keeps the "just move the mouse in" workflow working
    // without leaking scroll/zoom into the rest of the editor.
    if (m_ViewportHovered) m_EditorCamera.OnEvent(e);

    if (m_BlockEvents) {
        ImGuiIO& io  = ImGui::GetIO();
        bool handled = false;
        handled |= e.isInCategory(EventCategory_Mouse) & io.WantCaptureMouse;
        handled |= e.isInCategory(EventCategory_Keyboard) & io.WantCaptureKeyboard;
        e.SetHandled(handled);
    }
}

bool EditorLayer::OnKeyPressed(KeyPressedEvent& e) {
    // Engine-level shortcut: the layer stack dispatches key events here.
    CZ_EDITOR_LOG(Trace, "{}", e.ToString());

    return true;
}

void EditorLayer::Draw(CommandList cmdList) {

    m_ImGuiRenderer->Draw(ImGui::GetDrawData(), cmdList);
}

void EditorLayer::Init() {}

void EditorLayer::SetFont(std::string font) {

    // ImGuiIO& io       = ImGui::GetIO();

    // io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), fontSize * fbScale.Width /
}

void EditorLayer::SetDarkThemeColors() {
    auto& colors = ImGui::GetStyle().Colors;

    colors[ImGuiCol_WindowBg]  = ImVec4(0.22f, 0.22f, 0.24f, 1.00f);
    colors[ImGuiCol_ChildBg]   = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_PopupBg]   = ImVec4(0.06f, 0.06f, 0.08f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.06f, 0.06f, 0.08f, 1.00f);

    colors[ImGuiCol_TitleBg]          = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TitleBgActive]    = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);

    colors[ImGuiCol_Header]        = ImVec4(0.32f, 0.32f, 0.36f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.42f, 0.42f, 0.46f, 1.00f);
    colors[ImGuiCol_HeaderActive]  = ImVec4(0.46f, 0.46f, 0.50f, 1.00f);

    colors[ImGuiCol_Button]        = ImVec4(0.34f, 0.34f, 0.38f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.46f, 0.46f, 0.50f, 1.00f);
    colors[ImGuiCol_ButtonActive]  = ImVec4(0.42f, 0.42f, 0.46f, 1.00f);

    colors[ImGuiCol_FrameBg]        = ImVec4(0.26f, 0.26f, 0.28f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.32f, 0.32f, 0.36f, 1.00f);
    colors[ImGuiCol_FrameBgActive]  = ImVec4(0.36f, 0.36f, 0.40f, 1.00f);

    colors[ImGuiCol_Tab]                = ImVec4(0.28f, 0.28f, 0.32f, 1.00f);
    colors[ImGuiCol_TabHovered]         = ImVec4(0.42f, 0.42f, 0.46f, 1.00f);
    colors[ImGuiCol_TabActive]          = ImVec4(0.36f, 0.36f, 0.40f, 1.00f);
    colors[ImGuiCol_TabUnfocused]       = ImVec4(0.24f, 0.24f, 0.28f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.28f, 0.28f, 0.32f, 1.00f);

    colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.36f, 0.36f, 0.40f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.46f, 0.46f, 0.50f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.50f, 0.50f, 0.54f, 1.00f);

    colors[ImGuiCol_SliderGrab]       = ImVec4(0.40f, 0.40f, 0.44f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.48f, 0.48f, 0.52f, 1.00f);

    colors[ImGuiCol_CheckMark] = ImVec4(0.85f, 0.85f, 0.95f, 1.00f);

    colors[ImGuiCol_Separator]        = ImVec4(0.32f, 0.32f, 0.36f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.48f, 1.00f);
    colors[ImGuiCol_SeparatorActive]  = ImVec4(0.48f, 0.48f, 0.52f, 1.00f);

    colors[ImGuiCol_Text]         = ImVec4(0.98f, 0.98f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.70f, 0.70f, 0.74f, 1.00f);

    colors[ImGuiCol_Border]       = ImVec4(0.35f, 0.35f, 0.39f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    colors[ImGuiCol_DragDropTarget] = ImVec4(0.65f, 0.65f, 0.75f, 0.90f);

    colors[ImGuiCol_ResizeGrip]        = ImVec4(0.32f, 0.32f, 0.36f, 1.00f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.46f, 0.46f, 0.50f, 1.00f);
    colors[ImGuiCol_ResizeGripActive]  = ImVec4(0.50f, 0.50f, 0.54f, 1.00f);
}

void EditorLayer::NewProject() {}

void EditorLayer::OpenProject() {

    // UFileDialog::Get().Open(
    //     "TextureOpenDialog", "Open a texture",
    //     "Image file (*.png;*.jpg;*.jpeg;*.bmp;*.tga){.png,.jpg,.jpeg,.bmp,.tga},.*");
}

void EditorLayer::OpenProject(const std::filesystem::path& path) {}

void EditorLayer::SaveProjectAs() {}