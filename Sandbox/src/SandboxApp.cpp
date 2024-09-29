#include "SandboxApp.h"

ExampleLayer::ExampleLayer()
    : Layer("Example"),
        m_Camera(1.0f, 1.0f),
        m_CameraPosition(0.0f),
        m_SquarePosition(0.0f)
{
}

void ExampleLayer::OnAttach()
{
    m_CheckerboardTexture = Chozo::Texture2D::Create(std::string("../assets/textures/checkerboard.png"));
    m_OpenGLLogoTexture = Chozo::Texture2D::Create(std::string("../assets/textures/OpenGL_Logo.png"));
}

void ExampleLayer::OnDetach()
{
}

void ExampleLayer::OnUpdate(Chozo::Timestep ts)
{
}

void ExampleLayer::OnImGuiRender()
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
    ImGui::Begin("DockSpace Demo", nullptr, window_flags);
    ImGui::PopStyleVar();

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    }
    
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit")) Chozo::Application::Get().Close();
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::Begin("Settings");
    ImGui::Text("Renderer stats:");
    ImGui::ColorEdit3("SquareColor", glm::value_ptr(m_SquareColor));
  
    uint32_t textureID = m_CheckerboardTexture->GetRendererID();
    ImGui::Image((void*)(uintptr_t)textureID, ImVec2(64.0f, 64.0f));
    ImGui::End();

    ImGui::End();
}

void ExampleLayer::OnEvent(Chozo::Event &event)
{
}
