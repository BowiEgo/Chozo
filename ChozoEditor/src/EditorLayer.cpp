#include "EditorLayer.h"

namespace Chozo {

    EditorLayer::EditorLayer()
        : Layer("Example"),
            m_Camera(-1.6f, 1.6f, -0.9f, 0.9f),
            m_CameraPosition(0.0f),
            m_SquarePosition(0.0f)
    {
        /////////////////// Triangle ////////////////////////////////////
        m_TriangleVA.reset(Chozo::VertexArray::Create());

        float vertices[3 * 7] = {
            -0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
                0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
                0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
        };
        Chozo::Ref<Chozo::VertexBuffer> triangleVB;
        triangleVB.reset(Chozo::VertexBuffer::Create(vertices, sizeof(vertices)));

        Chozo::BufferLayout layout = {
            { Chozo::ShaderDataType::Float3, "a_Position" },
            { Chozo::ShaderDataType::Float4, "a_Color" },
        };
        triangleVB->SetLayout(layout);
        m_TriangleVA->AddVertexBuffer(triangleVB);
        
        unsigned int indices[3] = { 0, 1, 2 };
        Chozo::Ref<Chozo::IndexBuffer> triangleIB;
        triangleIB.reset(Chozo::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
        m_TriangleVA->SetIndexBuffer(triangleIB);

        /////////////////// Square ////////////////////////////////////
        m_SquareVA.reset(Chozo::VertexArray::Create());
        float squareVertices[5 * 4] = {
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
                0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
                0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
            -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
        };
        Chozo::Ref<Chozo::VertexBuffer> squareVB;
        squareVB.reset(Chozo::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
        squareVB->SetLayout({
            { Chozo::ShaderDataType::Float3, "a_Position" },
            { Chozo::ShaderDataType::Float2, "a_TexCoord" },
        });
        m_SquareVA->AddVertexBuffer(squareVB);

        unsigned int squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
        Chozo::Ref<Chozo::IndexBuffer> squareIB;
        squareIB.reset(Chozo::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
        m_SquareVA->SetIndexBuffer(squareIB);

        std::string vertexSrc = R"(
            #version 330 core

            layout(location = 0) in vec3 a_Position;
            layout(location = 1) in vec4 a_Color;

            uniform mat4 u_ViewProjection;
            uniform mat4 u_ModelMatrix;

            out vec3 v_Position;
            out vec4 v_Color;

            void main()
            {
                v_Position = a_Position;
                v_Color = a_Color;
                gl_Position = u_ViewProjection * u_ModelMatrix * vec4(a_Position, 1.0);
            }
        )";

        std::string fragmentSrc = R"(
            #version 330 core

            layout(location = 0) out vec4 color;

            in vec3 v_Position;
            in vec4 v_Color;

            void main()
            {
                color = vec4(v_Position * 0.5 + 0.5, 1.0);
                color = v_Color;
            }
        )";

        m_Shader = Chozo::Shader::Create("VertexPosColor", vertexSrc, fragmentSrc);

        std::string flatColorShaderVertexSrc = R"(
            #version 330 core

            layout(location = 0) in vec3 a_Position;

            uniform mat4 u_ViewProjection;
            uniform mat4 u_ModelMatrix;

            out vec3 v_Position;

            void main()
            {
                v_Position = a_Position;
                gl_Position = u_ViewProjection * u_ModelMatrix * vec4(a_Position, 1.0);
            }
        )";

        std::string flatColorShaderFragmentSrc = R"(
            #version 330 core

            layout(location = 0) out vec4 color;

            in vec3 v_Position;

            uniform vec3 u_Color;

            void main()
            {
                color = vec4(u_Color, 1.0);
            }
        )";

        m_flatColorShader = Chozo::Shader::Create("FlatColor", flatColorShaderVertexSrc, flatColorShaderFragmentSrc);

        auto textureShader = m_ShaderLibrary.Load("../assets/shaders/Texture.glsl");

        std::dynamic_pointer_cast<Chozo::OpenGLShader>(textureShader)->Bind();
        std::dynamic_pointer_cast<Chozo::OpenGLShader>(textureShader)->UploadUniformInt("u_Texture", 0);
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
        if (Chozo::Input::IsKeyPressed(CZ_KEY_LEFT))
            m_CameraPosition.x -= m_CameraMoveSpeed * ts;
        else if (Chozo::Input::IsKeyPressed(CZ_KEY_RIGHT))
            m_CameraPosition.x += m_CameraMoveSpeed * ts;

        if (Chozo::Input::IsKeyPressed(CZ_KEY_DOWN))
            m_CameraPosition.y -= m_CameraMoveSpeed * ts;
        else if (Chozo::Input::IsKeyPressed(CZ_KEY_UP))
            m_CameraPosition.y += m_CameraMoveSpeed * ts;

        if (Chozo::Input::IsKeyPressed(CZ_KEY_A))
            m_CameraRotation += m_CameraRotationSpeed * ts;
        if (Chozo::Input::IsKeyPressed(CZ_KEY_D))
            m_CameraRotation -= m_CameraRotationSpeed * ts;

        // Square control
        if (Chozo::Input::IsKeyPressed(CZ_KEY_J))
            m_SquarePosition.x -= m_SquareMoveSpeed * ts;
        else if (Chozo::Input::IsKeyPressed(CZ_KEY_L))
            m_SquarePosition.x += m_SquareMoveSpeed * ts;

        if (Chozo::Input::IsKeyPressed(CZ_KEY_K))
            m_SquarePosition.y -= m_SquareMoveSpeed * ts;
        else if (Chozo::Input::IsKeyPressed(CZ_KEY_I))
            m_SquarePosition.y += m_SquareMoveSpeed * ts;

        Chozo::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        Chozo::RenderCommand::Clear();

        m_Camera.SetPosition(m_CameraPosition);
        m_Camera.SetRotation(m_CameraRotation);

        Chozo::Renderer::BeginScene(m_Camera);

        // Square grid
        static glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

        glm::vec4 redColor(0.8f, 0.2f, 0.3f, 1.0f);
        glm::vec4 blueColor(0.2f, 0.3f, 0.8f, 1.0f);

        std::dynamic_pointer_cast<Chozo::OpenGLShader>(m_flatColorShader)->Bind();
        std::dynamic_pointer_cast<Chozo::OpenGLShader>(m_flatColorShader)->UploadUniformFloat3("u_Color", m_SquareColor);
        
        for (int y = -10; y < 10; y++)
        {
            for (int x = -10; x < 10; x++)
            {
                glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
                glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos + m_SquarePosition) * scale;
                Chozo::Renderer::Submit(m_flatColorShader, m_SquareVA, transform);
            }
        }

        auto textureShader = m_ShaderLibrary.Get("Texture");

        m_CheckerboardTexture->Bind();
        Chozo::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

        m_OpenGLLogoTexture->Bind();
        Chozo::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

        // Triangle
        Chozo::Renderer::Submit(m_Shader, m_TriangleVA);

        Chozo::Renderer::EndScene();

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
        ImGui::ColorEdit3("SquareColor", glm::value_ptr(m_SquareColor));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
        ImGui::Begin("Viewport");
        ImVec2 viewportPanelSize =  ImGui::GetContentRegionAvail();
        if (m_Viewport_FBO->GetWidth() != viewportPanelSize.x || m_Viewport_FBO->GetHeight() != viewportPanelSize.y)
        {
            m_Viewport_FBO->Resize((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y);
        }
        uint32_t textureID = m_Viewport_FBO->GetColorAttachmentRendererID();
        ImGui::Image((void*)(uintptr_t)textureID, ImVec2(m_Viewport_FBO->GetWidth(), m_Viewport_FBO->GetHeight()), ImVec2(0, 1), ImVec2(1, 0));
        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::End();

        ImGui::End();
    }

    void EditorLayer::OnEvent(Chozo::Event &event)
    {
    }
}
