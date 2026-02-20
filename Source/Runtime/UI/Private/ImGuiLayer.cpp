#include "ImGuiLayer.h"

#include "ApplicationEvent.h"
#include "ModuleUtils.h"
#include "VFS.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"

DEFINE_LOG_CATEGORY(LogImGuiLayer);

// inline FImVec2 operator+(const FImVec2& lhs, const FImVec2& rhs) {
//     return FImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
// }

// inline FImVec2 operator-(const FImVec2& lhs, const FImVec2& rhs) {
//     return FImVec2(lhs.x - rhs.x, lhs.y - rhs.y);
// }

CImGuiLayer::CImGuiLayer(CWindow* window, CRenderer* renderer)
    : ILayer("ImGuiLayer"), m_Window(window), m_Renderer(renderer) {
    m_ImGuiRenderer = CreateRenderer(window, m_Renderer->GetGraphicsContext());
}

CImGuiLayer::~CImGuiLayer() {}

TScope<IImGuiRenderer> CImGuiLayer::CreateRenderer(CWindow* window, CGraphicsContext* context) {

    if (m_RHIModule.Load("VulkanImGui.dll")) {
        m_RHIModule.Load(GetPlatformLibName("VulkanImGui"));
        return TScope<IImGuiRenderer>(
            m_RHIModule.Invoke<IImGuiRenderer*(CWindow*, CGraphicsContext*)>(
                "CreateVulkanImGuiRenderer", window, context));
    }

    CZ_LOG(LogImGuiLayer, Error, "Unsupported RendererAPI for ImGuiRenderer!");
    return nullptr;
}

void CImGuiLayer::OnAttach() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
    // io.ConfigViewportsNoAutoMerge = true; io.ConfigViewportsNoTaskBarIcon = true;

    SetFont("Titillium_Web/TitilliumWeb-Regular.ttf");

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look
    // identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.FrameRounding = 2.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // SetDarkThemeColors();

    // Setup Platform/Renderer backends
    m_ImGuiRenderer->Init(ImGui::GetCurrentContext());
}

void CImGuiLayer::OnDetach() { m_ImGuiRenderer->Shutdown(); }

void CImGuiLayer::OnImGuiRender() {}

void CImGuiLayer::OnEvent(IEvent& e) {
    if (m_BlockEvents) {
        ImGuiIO& io = ImGui::GetIO();
        bool handled = false;
        handled |= e.isInCategory(EventCategory_Mouse) & io.WantCaptureMouse;
        handled |= e.isInCategory(EventCategory_Keyboard) & io.WantCaptureKeyboard;
        e.SetHandled(handled);
    }

    if (typeid(e) == typeid(FWindowContentScaledEvent)) {
        auto& scaledEvent = static_cast<FWindowContentScaledEvent&>(e);
        auto dpi = scaledEvent.GetXScale();

        // SetFont("/fonts/Titillium_Web/TitilliumWeb-Regular.ttf", dpi);
        // ImGui_ImplOpenGL3_DestroyFontsTexture();
        // ImGui_ImplOpenGL3_CreateFontsTexture();
    }
}

void CImGuiLayer::Render(const TRef<IRHICommandBuffer>& cmdBuffer) {
    m_ImGuiRenderer->Render(ImGui::GetDrawData(), cmdBuffer);
}

void CImGuiLayer::Begin() {
    ImGuiIO& io = ImGui::GetIO();
    auto fbSize = m_Window->GetFramebufferSize();
    io.DisplaySize = ImVec2((float)fbSize.Width, (float)fbSize.Height);

    m_ImGuiRenderer->NewFrame();
}

void CImGuiLayer::End() {
    // Rendering
    ImGui::Render();

    auto drawData = ImGui::GetDrawData();

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void CImGuiLayer::SetFont(std::string font) {
    const auto dpi = m_Window->GetDPI();
    std::filesystem::path fontPath = VFS::Resolve("fonts://" + font);

    if (!std::filesystem::exists(fontPath)) {
        CZ_LOG(LogImGuiLayer, Error, "Font file not found: {}", fontPath.string());
        return;
    }

    ImGuiIO& io = ImGui::GetIO();

    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), 18.0f * dpi);
    io.FontDefault = io.Fonts->Fonts.back();
}

void CImGuiLayer::SetDarkThemeColors() {
    auto& colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = ImVec4{0.15f, 0.155f, 0.16f, 1.0f};

    // Headers
    colors[ImGuiCol_Header] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
    colors[ImGuiCol_HeaderHovered] = ImVec4{0.4f, 0.405f, 0.41f, 1.0f};
    colors[ImGuiCol_HeaderActive] = ImVec4{0.35f, 0.35f, 0.35f, 1.0f};

    // Buttons
    colors[ImGuiCol_Button] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
    colors[ImGuiCol_ButtonHovered] = ImVec4{0.38f, 0.385f, 0.39f, 1.0f};
    colors[ImGuiCol_ButtonActive] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};

    // Frame BG
    colors[ImGuiCol_FrameBg] = ImVec4{0.28f, 0.285f, 0.29f, 1.0f};
    colors[ImGuiCol_FrameBgHovered] = ImVec4{0.38f, 0.385f, 0.39f, 1.0f};
    colors[ImGuiCol_FrameBgActive] = ImVec4{0.10f, 0.105f, 0.11f, 1.0f};

    // Tabs
    colors[ImGuiCol_Tab] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
    colors[ImGuiCol_TabHovered] = ImVec4{0.38f, 0.385f, 0.39f, 1.0f};
    colors[ImGuiCol_TabActive] = ImVec4{0.28f, 0.285f, 0.29f, 1.0f};
    colors[ImGuiCol_TabUnfocused] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.1f, 0.105f, 0.11f, 1.0f};

    // Titles
    colors[ImGuiCol_TitleBg] = ImVec4{0.15f, 0.155f, 0.16f, 1.0f};
    colors[ImGuiCol_TitleBgActive] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.95f, 0.155f, 0.91f, 1.0f};
}
