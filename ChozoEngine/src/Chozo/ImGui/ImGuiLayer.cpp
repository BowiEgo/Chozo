#include "ImGuiLayer.h"

#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

#include "Chozo/Core/Application.h"

#include <GLFW/glfw3.h>

namespace Chozo {
    ImGuiLayer::ImGuiLayer()
        : Layer("ImGuiLayer")
    {
    }

    ImGuiLayer::~ImGuiLayer()
    {
    }

    void ImGuiLayer::OnAttach()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
        //io.ConfigViewportsNoAutoMerge = true;
        //io.ConfigViewportsNoTaskBarIcon = true;

        // io.FontDefault =  io.Fonts->AddFontFromFileTTF("../assets/fonts/Open_Sans/static/OpenSans-Regular.ttf", 16.0f);
        // io.FontDefault =  io.Fonts->AddFontFromFileTTF("../assets/fonts/Nunito/static/Nunito-Regular.ttf", 16.0f);
        // io.FontDefault =  io.Fonts->AddFontFromFileTTF("../assets/fonts/Nunito_Sans/static/NunitoSans_10pt-Regular.ttf", 16.0f);
        // io.FontDefault =  io.Fonts->AddFontFromFileTTF("../assets/fonts/Raleway/static/Raleway-Regular.ttf", 16.0f);
        // io.FontDefault =  io.Fonts->AddFontFromFileTTF("../assets/fonts/Roboto/Roboto-Regular.ttf", 16.0f);
        // io.FontDefault =  io.Fonts->AddFontFromFileTTF("../assets/fonts/Abel/Abel-Regular.ttf", 16.0f);
        // io.FontDefault =  io.Fonts->AddFontFromFileTTF("../assets/fonts/Nanum_Gothic/NanumGothic-Regular.ttf", 16.0f);
        io.Fonts->AddFontFromFileTTF("../assets/fonts/Titillium_Web/TitilliumWeb-Bold.ttf", 18.0f);
        io.FontDefault =  io.Fonts->AddFontFromFileTTF("../assets/fonts/Titillium_Web/TitilliumWeb-Regular.ttf", 18.0f);

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        SetDarkThemeColors();

        // Setup Platform/Renderer backends
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 410");
    }

    void ImGuiLayer::OnDetach()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::OnImGuiRender()
    {
    }

    void ImGuiLayer::OnEvent(Event &e)
    {
        if (m_BlockEvents)
        {
            ImGuiIO& io = ImGui::GetIO();
            bool handled = false;
            handled |= e.isInCategory(EventCategoryMouse) & io.WantCaptureMouse;
            handled |= e.isInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
            e.SetHandled(handled);
        }
    }

    void ImGuiLayer::Begin()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiLayer::End()
    {
        ImGuiIO& io = ImGui::GetIO();
        auto& app = Application::Get();
        io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }

    void ImGuiLayer::SetDarkThemeColors()
    {
        auto& colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_WindowBg] = ImVec4{ 0.15f, 0.155f, 0.16f, 1.0f };

        // Headers
        colors[ImGuiCol_Header] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_HeaderActive] = ImVec4{ 0.35f, 0.35f, 0.35f, 1.0f };

        // Buttons
        colors[ImGuiCol_Button] = ImVec4{ 0.28f, 0.285f, 0.29f, 1.0f };
        colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.38f, 0.385f, 0.39f, 1.0f };
        colors[ImGuiCol_ButtonActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

        // Frame BG
        colors[ImGuiCol_FrameBg] = ImVec4{ 0.28f, 0.285f, 0.29f, 1.0f };
        colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.38f, 0.385f, 0.39f, 1.0f };
        colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.10f, 0.105f, 0.11f, 1.0f };

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.385f, 0.39f, 1.0f };
        colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.285f, 0.29f, 1.0f };
        colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

        // Titles
        colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.155f, 0.16f, 1.0f };
        colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.95f, 0.155f, 0.91f, 1.0f };
    }
}
