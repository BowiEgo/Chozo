#include "EditorLayer.hpp"

#include "Core/Memory/Memory.hpp"
#include "UIUtils.hpp"

#include <Runtime/App/Application.hpp>
#include <Runtime/RHI/RHIAPI.hpp>

#include "../Runtime/Window/SDLWindow/SDLWindowObj.hpp"

using namespace CZ;

EditorLayer::EditorLayer() {}

EditorLayer::~EditorLayer() {}

void EditorLayer::OnAttach() {
    auto window = Application::Get().GetWindow();

    window.As<SDLWindowObj>()->SetEventPreprocessor(
        [](const SDL_Event& event) -> void { ImGui_ImplSDL3_ProcessEvent(&event); });

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

    m_ImGuiRenderer = CZ_CREATE_SCOPE(MEMORY_USAGE_UI, VulkanImGuiRenderer);
    m_ImGuiRenderer->Init(ImGui::GetCurrentContext(), window.As<SDLWindowObj>()->GetSDLWindow());

    m_ViewportRenderer = Application::Get().GetEngine()->GetRenderer();
    m_Viewport = m_ViewportRenderer.CreateViewport("Editor", m_ViewportSize.x, m_ViewportSize.y);

    // The viewport owns the scene; the editor only holds a view of it.
    m_Scene      = m_Viewport->GetScene();
    m_SyncBridge = CZ_CREATE_SCOPE(MEMORY_USAGE_UI, SyncBridge, m_Scene);

    auto mainCamera = m_Viewport->GetCamera();
    m_EditorCamera.SetActiveCamera(mainCamera);
    mainCamera->SetPerspective(45.0f, (float)fbSize.Width / fbSize.Height, 0.1f, 1000.0f);
    mainCamera->SetPosition(Vector3(0, 0, 5));

    CallbackHandle handle = m_NodeTree.RegisterEventCallback([this](const NodeEvent& event) {
        switch (event.GetType()) {
            case EditorNodeEventType::Created: m_SyncBridge->RegisterNode(event.GetNode()); break;
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

    m_ConsolePanel.Open();
    m_SceneHierarchyPanel.Open();
    m_PropertiesPanel.Open();

    {
        // Material

        auto cubeNodeMask   = GET_NODE_MASK("Node_Regular", "ProceduralMesh_Cube");
        EditorNode* newNode = m_NodeTree.CreateNode("Cube", cubeNodeMask, nullptr);
        m_NodeTree.SelectNode(newNode);

        // static_cast<FSphereParams*>(newNode->GetMeshParamsWrapper()->Get())->Material =
    }
}

void EditorLayer::OnDetach() {
    if (m_ImGuiRenderer) {
        m_ImGuiRenderer->ReleaseAllTextures();
        m_ImGuiRenderer->Shutdown();
    }
}

void EditorLayer::OnUpdate(float deltaTime) {
    m_Viewport->Resize(m_ViewportSize.x, m_ViewportSize.y);
    m_EditorCamera.OnUpdate(deltaTime, m_ViewportFocused);
    m_SyncBridge->SyncAllNodesToEntities();
}

void EditorLayer::OnRender() {
    m_ImGuiRenderer->NewFrame();

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

    ImGui::Render();

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void EditorLayer::OnEvent(Event& e) {
    if (m_BlockEvents) {
        ImGuiIO& io  = ImGui::GetIO();
        bool handled = false;
        handled |= e.isInCategory(EventCategory_Mouse) & io.WantCaptureMouse;
        handled |= e.isInCategory(EventCategory_Keyboard) & io.WantCaptureKeyboard;
        e.SetHandled(handled);
    }
}

bool EditorLayer::OnKeyPressed(KeyPressedEvent& e) {
    CZ_EDITOR_LOG(Trace, "{}}", e.ToString());

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