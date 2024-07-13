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
        : Layer("Example")
    {
        std::string vertexSrc = ReadFile("../assets/shaders/Shader.glsl.vert");
        std::string fragmentSrc = ReadFile("../assets/shaders/Shader.glsl.frag");

        m_Shader = Chozo::Shader::Create("VertexPosColor", vertexSrc, fragmentSrc);

        m_CameraController = std::make_unique<CameraController>();
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
        // --------------------
        // Scene
        // --------------------
        m_ActiveScene = std::make_shared<Scene>();
        Entity square = m_ActiveScene->CreateEntity("Orange Square");
        square.AddCompoent<SpriteRendererComponent>(glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));
        m_Square_Entity = square;
    }

    void EditorLayer::OnDetach()
    {
    }

    void EditorLayer::OnUpdate(Chozo::Timestep ts)
    {
        // Camera control
        m_CameraController->SetActive(m_Viewport_Focused && m_Viewport_Hovered);
        m_CameraController->Update(ts);

        // Render
        Renderer2D::ResetStats();
        m_Viewport_FBO->Bind();
        Chozo::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        Chozo::RenderCommand::Clear();

        Chozo::Renderer2D::BeginScene(m_CameraController->GetCamera());
        Renderer2D::Submit(m_Shader);
        Renderer2D::BeginBatch();

        // Square grid
        for (float y = -10.0f; y < 10.0f; y += 0.25f)
        {
            for (float x = -10.0f; x < 10.0f; x += 0.25f)
            {
                glm::vec4 color = { (x + 10.0f) / 20.0f, 0.2f, (y + 10.0f) / 20.0f, 1.0f };
                Renderer2D::DrawQuad({ x, y }, { 0.22f, 0.22f }, color);
            }
        }
        // Update scene
        m_ActiveScene->OnUpdate(ts);

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

        if (m_Square_Entity)
        {
            ImGui::Separator();
            auto& tag = m_Square_Entity.GetCompoent<TagComponent>().Tag;
            ImGui::Text("%s", tag.c_str());
            auto& squareColor = m_Square_Entity.GetCompoent<SpriteRendererComponent>().Color;
            ImGui::ColorEdit3("##OrangeSquare", glm::value_ptr(squareColor));
            ImGui::Separator();
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
        ImGui::Begin("Viewport");

        m_Viewport_Focused = ImGui::IsWindowFocused();
        m_Viewport_Hovered = ImGui::IsWindowHovered();
        Application::Get().GetImGuiLayer().BlockEvents(!m_Viewport_Focused || !m_Viewport_Hovered);

        ImVec2 viewportPanelSize =  ImGui::GetContentRegionAvail();
        if (m_Viewport_FBO->GetWidth() != viewportPanelSize.x || m_Viewport_FBO->GetHeight() != viewportPanelSize.y)
        {
            m_Viewport_FBO->Resize((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y);
            m_CameraController->GetCamera().Resize(viewportPanelSize.x, viewportPanelSize.y);
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
