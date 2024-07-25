#include "EditorLayer.h"
#include "CameraController.h"

#include <glad/glad.h>

#include "Chozo/Scene/SceneSerializer.h"
#include "Chozo/Utils/PlatformUtils.h"
#include "Chozo/Math/Math.h"

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
    }

    void EditorLayer::OnAttach()
    {
        m_CheckerboardTexture = Texture2D::Create("../assets/textures/checkerboard.png");
        m_OpenGLLogoTexture = Texture2D::Create("../assets/textures/OpenGL_Logo.png");
        // --------------------
        // Viewport
        // --------------------
        FramebufferSpecification fbSpec;
        fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
        m_Viewport_FBO = Framebuffer::Create(fbSpec);
        m_ID_FBO = Framebuffer::Create(fbSpec);
        // --------------------
        // Scene
        // --------------------
        m_ActiveScene = std::make_shared<Scene>();
        // --------------------
        // Editor Camera
        // --------------------
        m_EditorCamera = EditorCamera(30.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
#if 0
        // --------------------
        // Camera entity
        // --------------------
        m_Camera_A = m_ActiveScene->CreateEntity("Camera A");
        m_Camera_A.AddCompoent<CameraComponent>();
        m_Camera_A.GetCompoent<CameraComponent>().Primary = m_Camera_A_Is_Primary;
        m_Camera_A.GetCompoent<TransformComponent>().Translation.z = 6.0f;
        m_Camera_B = m_ActiveScene->CreateEntity("Camera B");
        m_Camera_B.AddCompoent<CameraComponent>();
        m_Camera_B.GetCompoent<CameraComponent>().Primary = !m_Camera_A_Is_Primary;
        m_Camera_B.GetCompoent<CameraComponent>().Camera.SetProjectionType(SceneCamera::ProjectionType::Orthographic);
        m_Camera_B.GetCompoent<CameraComponent>().Camera.SetOrthographicFarClip(100.0f);
        // --------------------
        // Square entity
        // --------------------
        m_Square_Entity = m_ActiveScene->CreateEntity("Orange Square");
        m_Square_Entity.AddCompoent<SpriteRendererComponent>(glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));

        auto greenSquare = m_ActiveScene->CreateEntity("Green Square");
        greenSquare.AddCompoent<SpriteRendererComponent>(glm::vec4(0.5f, 1.0f, 0.0f, 1.0f));
        greenSquare.GetCompoent<TransformComponent>().Translation.x = 3.0f;
        greenSquare.GetCompoent<TransformComponent>().Translation.z = -6.0f;
        // --------------------
        // Square grid entities
        // --------------------
        // for (float y = -10.0f; y < 10.0f; y += 0.25f)
        // {
        //     for (float x = -10.0f; x < 10.0f; x += 0.25f)
        //     {
        //         glm::vec4 color = { (x + 10.0f) / 20.0f, 0.2f, (y + 10.0f) / 20.0f, 1.0f };

        //         Entity entity = m_ActiveScene->CreateEntity("Grid Square");
        //         glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f)) * glm::scale(glm::mat4(1.0f), { 0.22f, 0.22f, 0.0f });
        //         entity.GetCompoent<TransformComponent>().Transform = transform;
        //         entity.AddCompoent<SpriteRendererComponent>(color);
        //     }
        // }
        // --------------------
        // Camera controller
        // --------------------
        m_Camera_A.AddCompoent<NativeScriptComponent>().Bind<CameraController>();
        m_Camera_B.AddCompoent<NativeScriptComponent>().Bind<CameraController>();
#endif
        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
    }

    void EditorLayer::OnDetach()
    {
    }

    void EditorLayer::OnUpdate(Timestep ts)
    {
        // Resize
        if (FramebufferSpecification spec = m_Viewport_FBO->GetSpecification();
            m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero size framebuffer is invalid
            (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
        {
            m_Viewport_FBO->Resize(m_ViewportSize.x, m_ViewportSize.y);
            // m_ID_FBO->Resize(m_ViewportSize.x, m_ViewportSize.y);
            m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
            m_ActiveScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
        }

        // Camera control
        if (m_Camera_A && m_Camera_A.HasComponent<NativeScriptComponent>())
        {
            auto nsc_A = m_Camera_A.GetCompoent<NativeScriptComponent>();
            if (nsc_A.Instance)
                static_cast<CameraController*>(nsc_A.Instance)->SetActive(m_ViewportFocused);
        }

        if (m_Camera_B && m_Camera_B.HasComponent<NativeScriptComponent>())
        {
            auto nsc_B = m_Camera_B.GetCompoent<NativeScriptComponent>();
            if (nsc_B.Instance)
                static_cast<CameraController*>(nsc_B.Instance)->SetActive(m_ViewportFocused);
        }

        m_EditorCamera.OnUpdate(ts);

        m_Viewport_FBO->Bind();
        Renderer2D::ResetStats();
        RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        RenderCommand::Clear();
        m_Viewport_FBO->ClearColorAttachmentBuffer(1, (void*)-1); // clear entity ID attachment to -1

        // Update scene
        m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);

        auto [mx, my] = ImGui::GetMousePos();
        mx -= m_ViewportBounds[0].x;
        my -= m_ViewportBounds[0].y;
        auto viewportWidth = m_ViewportBounds[1].x - m_ViewportBounds[0].x;
        auto viewportHeight = m_ViewportBounds[1].y - m_ViewportBounds[0].y;
        my = viewportHeight - my;

        if (mx >= 0 && my >= 0 && mx < viewportWidth && my < viewportHeight)
        {
            int pixelID = m_Viewport_FBO->ReadPixel(1, mx, my);
            m_Entity_Hovered = pixelID == -1 ? Entity() : Entity((entt::entity)pixelID, m_ActiveScene.get());
            // CZ_INFO("{0}-{1}, {2}", mx, my, pixelID);
        }

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
        ImGuiStyle& style = ImGui::GetStyle();
        float minWinSizeX = style.WindowMinSize.x;
        style.WindowMinSize.x = 350.0f;
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
        }
        style.WindowMinSize.x = minWinSizeX;
        
        // --------------------
        // Menu
        // --------------------
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New", "Ctrl+N"))
                    NewScene();
                if (ImGui::MenuItem("Open...", "Ctrl+O"))
                    OpenScene();
                if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
                    SaveSceneAs();                    

                if (ImGui::MenuItem("Quit")) Application::Get().Close();
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        // --------------------
        // SceneHierarchy panel
        // --------------------
        m_SceneHierarchyPanel.OnImGuiRender();

        // --------------------
        // Settings panel
        // --------------------
        ImGui::Begin("Settings");
        ImGui::Text("Renderer stats:");
        ImGui::Text("DrawCalls: %d", Renderer2D::GetStats().DrawCalls);
        ImGui::Text("QuadCount: %d", Renderer2D::GetStats().QuadCount);
        ImGui::Text("Vertices: %d", Renderer2D::GetStats().GetTotalVertexCount());
        ImGui::Text("Indices: %d", Renderer2D::GetStats().GetTotalIndexCount());

        std::string entityName = "Null";
        if (m_Entity_Hovered)
            entityName = m_Entity_Hovered.GetCompoent<TagComponent>().Tag;
        ImGui::Text("EntityHoverd: %s", entityName.c_str());
        ImGui::Separator();

        // --------------------
        // Viewport
        // --------------------
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
        ImGui::Begin("Viewport");

        m_ViewportFocused = ImGui::IsWindowFocused();
        m_ViewportHovered = ImGui::IsWindowHovered();
        Application::Get().GetImGuiLayer().BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

        auto viewportOffset = ImGui::GetCursorPos(); // includes tab bar
        m_ViewportSize = ImGui::GetContentRegionAvail();
    
        uint32_t textureID = m_Viewport_FBO->GetColorAttachmentRendererID(0);
        ImGui::Image((void*)(uintptr_t)textureID, m_ViewportSize, ImVec2(0, 1), ImVec2(1, 0));

        // Viewport bounds
        auto windowSize = ImGui::GetWindowSize();
        ImVec2 minBound = ImGui::GetWindowPos();
        minBound.x += viewportOffset.x;
        minBound.y += viewportOffset.y;

        ImVec2 maxBound = { minBound.x + windowSize.x, minBound.y + windowSize.y };
        m_ViewportBounds[0] = { minBound.x, minBound.y };
        m_ViewportBounds[1] = { maxBound.x, maxBound.y };
        m_AllowViewportCameraEvents = ImGui::IsMouseHoveringRect(minBound, maxBound);

        // Gizmos
        Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
        if (selectedEntity)
        {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();

            float windowWidth = ImGui::GetWindowWidth();
            float windowHeight = ImGui::GetWindowHeight();
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

            // Camera;

            // Runtime camera from entity
            // auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
            // const auto& camera = cameraEntity.GetCompoent<CameraComponent>().Camera;
            // const glm::mat4 cameraProjection = camera.GetProjection();
            // const glm::mat4 cameraView = glm::inverse(cameraEntity.GetCompoent<TransformComponent>().GetTransform());

            // Editor camera
            const glm::mat4 cameraProjection = m_EditorCamera.GetProjection();
            const glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

            // Entity transform
            auto& tc = selectedEntity.GetCompoent<TransformComponent>();
            glm::mat4 transform = tc.GetTransform();

            // Snapping
            bool snap = Input::IsKeyPressed(CZ_KEY_LEFT_CONTROL);
            float snapValue = 0.5f; // Snap to 0.5m for translation/scale
            // Snap to 45 degrees for rotation
            if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
                snapValue = 45.0f;
            
            float snapValues[3] = { snapValue, snapValue, snapValue };

            ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                ImGuizmo::OPERATION(m_GizmoType), ImGuizmo::LOCAL, glm::value_ptr(transform),
                nullptr, snap ? snapValues : nullptr);

            if (ImGuizmo::IsUsing())
            {
                glm::vec3 translation, rotation, scale;
                Math::DecomposeTransform(transform, translation, rotation, scale);
                
                glm::vec3 deltaRotation = rotation - tc.Rotation;
                tc.Translation = translation;
                tc.Rotation += deltaRotation;
                tc.Scale = scale;
            }
        }

        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::End();

        ImGui::End();
    }

    void EditorLayer::OnEvent(Event &e)
    {
        // m_CameraController->OnEvent(e);
        m_EditorCamera.OnEvent(e);
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<KeyPressedEvent>(CZ_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
        dispatcher.Dispatch<MouseButtonPressedEvent>(CZ_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
    }

    bool EditorLayer::OnKeyPressed(KeyPressedEvent &e)
    {
        // Shortcuts
        if (e.GetRepeatCount() > 0)
            return false;

        bool control = Input::IsKeyPressed(CZ_KEY_LEFT_CONTROL) || Input::IsKeyPressed(CZ_KEY_RIGHT_CONTROL);
        bool shift = Input::IsKeyPressed(CZ_KEY_LEFT_SHIFT) || Input::IsKeyPressed(CZ_KEY_LEFT_SHIFT);
        switch (e.GetKeyCode())
        {
            case Key::N:
            {
                if (control)
                    NewScene();
                break;
            }
            case Key::O:
            {
                if (control)
                    OpenScene();
                break;
            }
            case Key::S:
            {
                if (control && shift)
                    SaveSceneAs();
                break;
            }

            // Gizmos
            case Key::Q:
                m_GizmoType = -1;
                break;
            case Key::W:
                m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
                break;
            case Key::E:
                m_GizmoType = ImGuizmo::OPERATION::ROTATE;
                break;
            case Key::R:
                m_GizmoType = ImGuizmo::OPERATION::SCALE;
                break;
            default:
                break;
        }
        return true;
    }

    bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent &e)
    {
        if (e.GetMouseButton() == MouseButton::Left && !Input::IsKeyPressed(Key::LeftAlt) && !ImGuizmo::IsUsing() && !ImGuizmo::IsOver() && m_ViewportHovered)
        {
            m_SceneHierarchyPanel.SetSelectedEntity(m_Entity_Hovered);

            return true;
        }

        return false;
    }

    std::pair<float, float> EditorLayer::GetMouseViewportSpace()
    {
        auto [mx, my] = ImGui::GetMousePos();
        mx -= m_ViewportBounds[0].x;
        my -= m_ViewportBounds[0].y;
        auto viewportWidth = m_ViewportBounds[1].x - m_ViewportBounds[0].x;
		auto viewportHeight = m_ViewportBounds[1].y - m_ViewportBounds[0].y;

		return { (mx / viewportWidth) * 2.0f - 1.0f, ((my / viewportHeight) * 2.0f - 1.0f) * -1.0f };
    }

    void EditorLayer::NewScene()
    {
        m_ActiveScene = std::make_shared<Scene>();
        m_ActiveScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
    }

    void EditorLayer::OpenScene()
    {
        std::string filepath = FileDialogs::OpenFile("Chozo Scene (*.chozo)\0*.chozo\0");
        if (!filepath.empty())
        {
            m_ActiveScene = std::make_shared<Scene>();
            m_ActiveScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
            m_SceneHierarchyPanel.SetContext(m_ActiveScene);

            SceneSerializer serializer(m_ActiveScene);
            serializer.Deserialize(filepath);
        }
    }

    void EditorLayer::SaveSceneAs()
    {
        std::string filepath = FileDialogs::SaveFile("Chozo Scene (*.chozo)\0*.chozo\0");
        if (!filepath.empty())
        {
            SceneSerializer serializer(m_ActiveScene);
            serializer.Serialize(filepath);
        }
    }
}
