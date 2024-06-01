#include "Parallax.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class ExampleLayer : public Parallax::Layer
{
public:
    ExampleLayer()
        : Layer("Example"),
        m_Camera(-1.6f, 1.6f, -0.9f, 0.9f),
        m_CameraPosition(0.0f),
        m_SquarePosition(0.0f)
    {
        /////////////////// Triangle ////////////////////////////////////
        m_TriangleVA.reset(Parallax::VertexArray::Create());

        float vertices[3 * 7] = {
            -0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
             0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
             0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
        };
        Parallax::Ref<Parallax::VertexBuffer> triangleVB;
        triangleVB.reset(Parallax::VertexBuffer::Create(vertices, sizeof(vertices)));

        Parallax::BufferLayout layout = {
            { Parallax::ShaderDataType::Float3, "a_Position" },
            { Parallax::ShaderDataType::Float4, "a_Color" },
        };
        triangleVB->SetLayout(layout);
        m_TriangleVA->AddVertexBuffer(triangleVB);
        
        unsigned int indices[3] = { 0, 1, 2 };
        Parallax::Ref<Parallax::IndexBuffer> triangleIB;
        triangleIB.reset(Parallax::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
        m_TriangleVA->SetIndexBuffer(triangleIB);

        /////////////////// Square ////////////////////////////////////
        m_SquareVA.reset(Parallax::VertexArray::Create());
        float squareVertices[5 * 4] = {
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
             0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
             0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
            -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
        };
        Parallax::Ref<Parallax::VertexBuffer> squareVB;
        squareVB.reset(Parallax::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
        squareVB->SetLayout({
            { Parallax::ShaderDataType::Float3, "a_Position" },
            { Parallax::ShaderDataType::Float2, "a_TexCoord" },
        });
        m_SquareVA->AddVertexBuffer(squareVB);

        unsigned int squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
        Parallax::Ref<Parallax::IndexBuffer> squareIB;
        squareIB.reset(Parallax::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
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

        m_Shader.reset(Parallax::Shader::Create(vertexSrc, fragmentSrc));
    
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

        m_flatColorShader.reset(Parallax::Shader::Create(flatColorShaderVertexSrc, flatColorShaderFragmentSrc));

        m_TextureShader.reset(Parallax::Shader::Create("../assets/shaders/Texture.glsl"));

        m_Texture = Parallax::Texture2D::Create("../assets/textures/checkerboard.png");
        m_OpenGLLogoTexture = Parallax::Texture2D::Create("../assets/textures/OpenGL_Logo.png");

        std::dynamic_pointer_cast<Parallax::OpenGLShader>(m_TextureShader)->Bind();
        std::dynamic_pointer_cast<Parallax::OpenGLShader>(m_TextureShader)->UploadUniformInt("u_Texture", 0);
    }

    void OnUpdate(Parallax::Timestep ts) override
    {
        // Camera control
        if (Parallax::Input::IsKeyPressed(PRX_KEY_LEFT))
            m_CameraPosition.x -= m_CameraMoveSpeed * ts;
        else if (Parallax::Input::IsKeyPressed(PRX_KEY_RIGHT))
            m_CameraPosition.x += m_CameraMoveSpeed * ts;

        if (Parallax::Input::IsKeyPressed(PRX_KEY_DOWN))
            m_CameraPosition.y -= m_CameraMoveSpeed * ts;
        else if (Parallax::Input::IsKeyPressed(PRX_KEY_UP))
            m_CameraPosition.y += m_CameraMoveSpeed * ts;

        if (Parallax::Input::IsKeyPressed(PRX_KEY_A))
            m_CameraRotation += m_CameraRotationSpeed * ts;
        if (Parallax::Input::IsKeyPressed(PRX_KEY_D))
            m_CameraRotation -= m_CameraRotationSpeed * ts;

        // Square control
        if (Parallax::Input::IsKeyPressed(PRX_KEY_J))
            m_SquarePosition.x -= m_SquareMoveSpeed * ts;
        else if (Parallax::Input::IsKeyPressed(PRX_KEY_L))
            m_SquarePosition.x += m_SquareMoveSpeed * ts;

        if (Parallax::Input::IsKeyPressed(PRX_KEY_K))
            m_SquarePosition.y -= m_SquareMoveSpeed * ts;
        else if (Parallax::Input::IsKeyPressed(PRX_KEY_I))
            m_SquarePosition.y += m_SquareMoveSpeed * ts;

        Parallax::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        Parallax::RenderCommand::Clear();

        m_Camera.SetPosition(m_CameraPosition);
        m_Camera.SetRotation(m_CameraRotation);

        Parallax::Renderer::BeginScene(m_Camera);

        // Square grid
        static glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

        glm::vec4 redColor(0.8f, 0.2f, 0.3f, 1.0f);
        glm::vec4 blueColor(0.2f, 0.3f, 0.8f, 1.0f);

        std::dynamic_pointer_cast<Parallax::OpenGLShader>(m_flatColorShader)->Bind();
        std::dynamic_pointer_cast<Parallax::OpenGLShader>(m_flatColorShader)->UploadUniformFloat3("u_Color", m_SquareColor);
        
        for (int y = -10; y < 10; y++)
        {
            for (int x = -10; x < 10; x++)
            {
                glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
                glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos + m_SquarePosition) * scale;
                Parallax::Renderer::Submit(m_flatColorShader, m_SquareVA, transform);
            }
        }

        m_Texture->Bind();
        Parallax::Renderer::Submit(m_TextureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

        m_OpenGLLogoTexture->Bind();
        Parallax::Renderer::Submit(m_TextureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

        // Triangle
        // Parallax::Renderer::Submit(m_Shader, m_TriangleVA);

        Parallax::Renderer::EndScene();
    }

    virtual void OnImGuiRender() override
    {
        ImGui::Begin("Settings");
        ImGui::ColorEdit3("SquareColor", glm::value_ptr(m_SquareColor));
        ImGui::End();
    }

    void OnEvent(Parallax::Event& event) override
    {
    }

private:
    Parallax::Ref<Parallax::Shader> m_Shader;
    Parallax::Ref<Parallax::VertexArray> m_TriangleVA;

    Parallax::Ref<Parallax::Shader> m_flatColorShader, m_TextureShader;
    Parallax::Ref<Parallax::VertexArray> m_SquareVA;

    Parallax::Ref<Parallax::Texture2D> m_Texture, m_OpenGLLogoTexture;

    Parallax::OrthographicCamera m_Camera;
    glm::vec3 m_CameraPosition;
    float m_CameraMoveSpeed = 2.0f;
    float m_CameraRotation = 0.0f;
    float m_CameraRotationSpeed = 180.0f;

    glm::vec3 m_SquarePosition;
    float m_SquareMoveSpeed = 1.0f;

    glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };
};

class Sandbox : public Parallax::Application
{
public:
    Sandbox()
    {
        PRX_INFO("Welcome to Parallax Sandbox!");
        PushLayer(new ExampleLayer());
    }

    ~Sandbox()
    {
    }
};

Parallax::Application *Parallax::CreateApplication()
{
    return new Sandbox();
};
