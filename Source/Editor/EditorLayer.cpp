#include "EditorLayer.h"

#include "../Runtime/Window/SDLWindow/SDLWindowObj.h"

#include <Runtime/App/Application.h>
#include <Runtime/RHI/RHIAPI.h>

using namespace CZ;

extern "C" {
Layer* CreateEditorLayer() { return CZ_NEW(MEMORY_USAGE_RENDER, EditorLayer); }
}

DEFINE_LOG_CATEGORY(LogEditorLayer);

EditorLayer::EditorLayer() {}

EditorLayer::~EditorLayer() {}

void EditorLayer::OnAttach() {
    auto window = Application::Get().GetWindow();

    static_cast<SDLWindowObj*>(window.Unwrap())
        ->SetEventPreprocessor(
            [](const SDL_Event& event) -> void { ImGui_ImplSDL3_ProcessEvent(&event); });

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport
    // io.ConfigViewportsNoAutoMerge = true; io.ConfigViewportsNoTaskBarIcon = true;

    // SetFont("Titillium_Web/TitilliumWeb-Regular.ttf");

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
    // ImGui::StyleColorsLight();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look
    // identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding              = 0.0f;
        style.FrameRounding               = 2.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    float pixelRatio = window.GetPixelRatio();
    Extent2D fbScale = window.GetFrameBufferScale();
    style.ScaleAllSizes(fbScale.Width / pixelRatio);

    SetDarkThemeColors();

    m_ImGuiRenderer = CZ_CREATE_SCOPE(MEMORY_USAGE_UI, VulkanImGuiRenderer);
    m_ImGuiRenderer->Init(ImGui::GetCurrentContext(), (SDL_Window*)window.GetWindowWrapper());
}

void EditorLayer::OnDetach() { m_ImGuiRenderer->Shutdown(); }

void EditorLayer::OnUpdate(float deltaTime) {
    CZ_LOG(LogEditorLayer, Trace, "OnUpdate: {}", deltaTime);
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

    // for (auto Layer : m_Layers)
    //     Layer.OnRender();

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
    CZ_LOG(LogEditorLayer, Trace, "{}}", e.ToString());

    return true;
}

void EditorLayer::Draw() {
    auto drawData = ImGui::GetDrawData();

    if (!drawData || drawData->TotalVtxCount == 0) return;

    // auto vlkCmdBuffer     = cmdList.As<CVulkanCommandBuffer>();
    // vk::CommandBuffer cmd = vlkCmdBuffer->GetVKCommandBuffer();

    // ImGui_ImplVulkan_RenderDrawData(drawData, cmd);
}

void EditorLayer::Init() {}

void EditorLayer::SetFont(std::string font) {
    // std::filesystem::path fontPath = VFS::Resolve("fonts://" + font);

    // if (!std::filesystem::exists(fontPath)) {
    //     CZ_LOG(LogImGuiLayer, Error, "Font file not found: {}", fontPath.string());
    //     return;
    // }

    // ImGuiIO& io       = ImGui::GetIO();
    // float fontSize    = 18.0f;
    // float pixelRatio  = m_Window->GetPixelRatio();
    // FExtent2D fbScale = m_Window->GetFrameBufferScale();

    // io.Fonts->Clear();
    // io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), fontSize * fbScale.Width /
    // pixelRatio); io.FontDefault = io.Fonts->Fonts.back();
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
