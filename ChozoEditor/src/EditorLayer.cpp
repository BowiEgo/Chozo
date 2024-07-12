#include "EditorLayer.h"

#include <glad/glad.h>

namespace Chozo {

    std::string ReadFile(const std::string &filepath)
    {
        std::string result;
        std::ifstream in(filepath, std::ios::in | std::ios::binary);
        if (in)
        {
            in.seekg(0, std::ios::end);
            result.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(&result[0], result.size());
            in.close();
        }
        else
        {
            CZ_CORE_ASSERT("Could not open file '{0}'", filepath);
        }

        return result;
    }

    EditorLayer::EditorLayer()
        : Layer("Example"),
            m_Camera(1280.0f, 720.0f)
    {
        std::string vertexSrc = ReadFile("../assets/shaders/Shader.glsl.vert");
        std::string fragmentSrc = ReadFile("../assets/shaders/Shader.glsl.frag");

        m_Shader = Chozo::Shader::Create("VertexPosColor", vertexSrc, fragmentSrc);

        m_CameraController = std::make_unique<CameraController>(&m_Camera);
    }

    void EditorLayer::OnAttach()
    {
        m_CheckerboardTexture = Chozo::Texture2D::Create("../assets/textures/checkerboard.png");
        m_OpenGLLogoTexture = Chozo::Texture2D::Create("../assets/textures/OpenGL_Logo.png");
        // --------------------
        // Viewport
        // --------------------
        Chozo::FramebufferSpecification fbSpec;
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        m_Viewport_FBO = Chozo::Framebuffer::Create(fbSpec);
    }

    void EditorLayer::OnDetach()
    {
    }

    void EditorLayer::OnUpdate(Chozo::Timestep ts)
    {
        m_Viewport_FBO->Bind();

        // Camera control
        m_CameraController->Enable(m_Viewport_Focused && m_Viewport_Hovered);
        m_CameraController->Update(ts);

        Chozo::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        Chozo::RenderCommand::Clear();

        Chozo::Renderer2D::BeginScene(m_Camera);

        // Square grid
        std::dynamic_pointer_cast<Chozo::OpenGLShader>(m_Shader)->Bind();
        std::dynamic_pointer_cast<Chozo::OpenGLShader>(m_Shader)->UploadUniformFloat3("u_Color", m_SquareColor);
        Renderer2D::Submit(m_Shader);

        Renderer2D::ResetStats();
        Renderer2D::BeginBatch();
        for (float y = -500.0f; y < 500.0f; y += 25.0f)
        {
            for (float x = -500.0f; x < 500.0f; x += 25.0f)
            {
                glm::vec4 color = { (x + 500.0f) / 1000.0f, 0.2f, (y + 500.0f) / 1000.0f, 1.0f };
                Renderer2D::DrawQuad({ x, y }, { 22.0f, 22.0f }, color);
            }
        }

        Renderer2D::DrawQuad({0.0f, 0.0f}, { 100.0f, 100.0f }, glm::vec4(m_SquareColor, 1.0));
        Renderer2D::EndBatch();

        m_Viewport_FBO->Unbind();
    }

    void EditorLayer::OnImGuiRender()
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
                if (ImGui::MenuItem("Quit")) Chozo::Application::Get().Close();
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        ImGui::Begin("Settings");
        ImGui::Text("Renderer stats:");
        ImGui::Text("DrawCalls: %d", Renderer2D::GetStats().DrawCalls);
        ImGui::Text("QuadCount: %d", Renderer2D::GetStats().QuadCount);
        ImGui::Text("Vertices: %d", Renderer2D::GetStats().GetTotalVertexCount());
        ImGui::Text("Indices: %d", Renderer2D::GetStats().GetTotalIndexCount());
        ImGui::ColorEdit3("SquareColor", glm::value_ptr(m_SquareColor));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
        ImGui::Begin("Viewport");
        m_Viewport_Focused = ImGui::IsWindowFocused();
        m_Viewport_Hovered = ImGui::IsWindowHovered();

        ImVec2 viewportPanelSize =  ImGui::GetContentRegionAvail();
        if (m_Viewport_FBO->GetWidth() != viewportPanelSize.x || m_Viewport_FBO->GetHeight() != viewportPanelSize.y)
        {
            m_Viewport_FBO->Resize((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y);
            m_Camera.Resize(viewportPanelSize.x, viewportPanelSize.y);
        }
        uint32_t textureID = m_Viewport_FBO->GetColorAttachmentRendererID();
        ImGui::Image((void*)(uintptr_t)textureID, ImVec2(m_Viewport_FBO->GetWidth(), m_Viewport_FBO->GetHeight()), ImVec2(0, 1), ImVec2(1, 0));
        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::End();

        ImGui::End();
    }

    void EditorLayer::OnEvent(Chozo::Event &e)
    {
        m_CameraController->OnEvent(e);
    }
}
