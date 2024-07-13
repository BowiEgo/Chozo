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

        m_Shader = Shader::Create("VertexPosColor", vertexSrc, fragmentSrc);

        m_CameraController = std::make_unique<CameraController>();
    }

    void EditorLayer::OnAttach()
    {
        m_CheckerboardTexture = Texture2D::Create("../assets/textures/checkerboard.png");
        m_OpenGLLogoTexture = Texture2D::Create("../assets/textures/OpenGL_Logo.png");
        // --------------------
        // Viewport
        // --------------------
        FramebufferSpecification fbSpec;
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        m_Viewport_FBO = Framebuffer::Create(fbSpec);
        // --------------------
        // Scene
        // --------------------
        m_ActiveScene = std::make_shared<Scene>();
        // --------------------
        // Camera entity
        // --------------------
        m_Camera_A = m_ActiveScene->CreateEntity("Camera A");
        m_Camera_A.AddCompoent<CameraComponent>(glm::ortho(-12.8f, 7.2f, -12.8f, 7.2f));
        m_Camera_B = m_ActiveScene->CreateEntity("Camera B");
        m_Camera_B.AddCompoent<CameraComponent>(glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f));
        // --------------------
        // Square entity
        // --------------------
        m_Square_Entity = m_ActiveScene->CreateEntity("Orange Square");
        m_Square_Entity.AddCompoent<SpriteRendererComponent>(glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));
        // --------------------
        // Square grid entities
        // --------------------
        for (float y = -10.0f; y < 10.0f; y += 0.25f)
        {
            for (float x = -10.0f; x < 10.0f; x += 0.25f)
            {
                glm::vec4 color = { (x + 10.0f) / 20.0f, 0.2f, (y + 10.0f) / 20.0f, 1.0f };

                Entity entity = m_ActiveScene->CreateEntity("Grid Square");
                glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f)) * glm::scale(glm::mat4(1.0f), { 0.22f, 0.22f, 0.0f });
                entity.GetCompoent<TransformComponent>().Transform = transform;
                entity.AddCompoent<SpriteRendererComponent>(color);
            }
        }
    }

    void EditorLayer::OnDetach()
    {
    }

    void EditorLayer::OnUpdate(Timestep ts)
    {
        // Resize
        if (FramebufferSpecification spec = m_Viewport_FBO->GetSpecification();
            m_Viewport_Size.x > 0.0f && m_Viewport_Size.y > 0.0f && // zero size framebuffer is invalid
            (spec.Width != m_Viewport_Size.x || spec.Height != m_Viewport_Size.y))
        {
            m_Viewport_FBO->Resize(m_Viewport_Size.x, m_Viewport_Size.y);
            m_CameraController->Resize(m_Viewport_Size.x, m_Viewport_Size.y);
        }

        // Camera control
        m_CameraController->SetActive(m_Viewport_Focused && m_Viewport_Hovered);
        m_CameraController->Update(ts);

        m_Viewport_FBO->Bind();
        Renderer2D::ResetStats();
        RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        RenderCommand::Clear();

        Renderer2D::Submit(m_Shader);
        // Update scene
        m_ActiveScene->OnUpdate(ts);

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
                if (ImGui::MenuItem("Quit")) Application::Get().Close();
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

        ImGui::DragFloat3("Camera A Transform", glm::value_ptr(m_Camera_A.GetCompoent<TransformComponent>().Transform[3]));
        ImGui::Checkbox("Camera A", &m_Camera_A_Is_Primary);
        m_Camera_A.GetCompoent<CameraComponent>().Primary = m_Camera_A_Is_Primary;
        m_Camera_B.GetCompoent<CameraComponent>().Primary = !m_Camera_A_Is_Primary;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
        ImGui::Begin("Viewport");

        m_Viewport_Focused = ImGui::IsWindowFocused();
        m_Viewport_Hovered = ImGui::IsWindowHovered();
        Application::Get().GetImGuiLayer().BlockEvents(!m_Viewport_Focused || !m_Viewport_Hovered);

        m_Viewport_Size = ImGui::GetContentRegionAvail();
    
        uint32_t textureID = m_Viewport_FBO->GetColorAttachmentRendererID();
        ImGui::Image((void*)(uintptr_t)textureID, ImVec2(m_Viewport_Size.x, m_Viewport_Size.y), ImVec2(0, 1), ImVec2(1, 0));
        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::End();

        ImGui::End();
    }

    void EditorLayer::OnEvent(Event &e)
    {
        m_CameraController->OnEvent(e);
    }
}
