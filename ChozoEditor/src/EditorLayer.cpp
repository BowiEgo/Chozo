#include "EditorLayer.h"
#include "CameraController.h"

#include "Panels/TexturePreviewPanel.h"

#include <regex>

#include <glad/glad.h>

#include "Chozo/Scene/SceneSerializer.h"
#include "Chozo/Utils/PlatformUtils.h"
#include "Chozo/Math/Math.h"

namespace Chozo {

    extern const std::filesystem::path g_AssetsPath;
    extern const std::regex imagePattern;
    extern const std::regex scenePattern;

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
        m_IconPlay = Texture2D::Create("../resources/icons/Toolbar/play.png");
        m_IconStop = Texture2D::Create("../resources/icons/Toolbar/stop.png");
        // --------------------
        // Viewport
        // --------------------
        FramebufferSpecification fbSpec;
        fbSpec.Attachments = { ImageFormat::RGBA8, ImageFormat::RED32I, ImageFormat::Depth };
        m_Viewport_FBO = Framebuffer::Create(fbSpec);
        // m_ID_FBO = Framebuffer::Create(fbSpec);

        PipelineSpecification pipelineSpec;
        pipelineSpec.TargetFramebuffer = m_Viewport_FBO;
        m_Pipeline = Pipeline::Create(pipelineSpec);
        // --------------------
        // Scene
        // --------------------
        m_ActiveScene = std::make_shared<Scene>();
        m_ActiveScene->SetFinalPipeline(m_Pipeline);
		m_ViewportRenderer = SceneRenderer::Create(m_ActiveScene);
        m_ViewportRenderer->SetActive(true);
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
        m_Camera_A.GetComponent<CameraComponent>().Primary = m_Camera_A_Is_Primary;
        m_Camera_A.GetComponent<TransformComponent>().Translation.z = 6.0f;
        m_Camera_B = m_ActiveScene->CreateEntity("Camera B");
        m_Camera_B.AddCompoent<CameraComponent>();
        m_Camera_B.GetComponent<CameraComponent>().Primary = !m_Camera_A_Is_Primary;
        m_Camera_B.GetComponent<CameraComponent>().Camera.SetProjectionType(SceneCamera::ProjectionType::Orthographic);
        m_Camera_B.GetComponent<CameraComponent>().Camera.SetOrthographicFarClip(100.0f);
        // --------------------
        // Square entity
        // --------------------
        m_Square_Entity = m_ActiveScene->CreateEntity("Orange Square");
        m_Square_Entity.AddCompoent<SpriteRendererComponent>(glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));

        auto greenSquare = m_ActiveScene->CreateEntity("Green Square");
        greenSquare.AddCompoent<SpriteRendererComponent>(glm::vec4(0.5f, 1.0f, 0.0f, 1.0f));
        greenSquare.GetComponent<TransformComponent>().Translation.x = 3.0f;
        greenSquare.GetComponent<TransformComponent>().Translation.z = -6.0f;
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
        //         entity.GetComponent<TransformComponent>().Transform = transform;
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
        m_EnvironmentPanel.SetContext(m_ActiveScene);
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
            m_ViewportRenderer->SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
        }

        // Camera control
        if (m_Camera_A && m_Camera_A.HasComponent<NativeScriptComponent>())
        {
            auto nsc_A = m_Camera_A.GetComponent<NativeScriptComponent>();
            if (nsc_A.Instance)
                static_cast<CameraController*>(nsc_A.Instance.get())->SetActive(m_ViewportFocused);
        }

        if (m_Camera_B && m_Camera_B.HasComponent<NativeScriptComponent>())
        {
            auto nsc_B = m_Camera_B.GetComponent<NativeScriptComponent>();
            if (nsc_B.Instance)
                static_cast<CameraController*>(nsc_B.Instance.get())->SetActive(m_ViewportFocused);
        }

        // m_Viewport_FBO->Bind();
        // Renderer2D::ResetStats();
        // Renderer::ResetStats();
        // RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        // RenderCommand::Clear();
        // m_Viewport_FBO->ClearColorAttachmentBuffer(1, (void*)-1); // clear entity ID attachment to -1

        // Update scene
        switch (m_SceneState)
        {
            case SceneState::Edit:
            {
                if (m_AllowViewportCameraEvents)
                    m_EditorCamera.OnUpdate(ts);
                m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);
                break;
            }
            case SceneState::Play:
            {
                m_ActiveScene->OnUpdateRuntime(ts);
                break;
            }
        }

        auto [mx, my] = ImGui::GetMousePos();
        mx -= m_ViewportBounds[0].x;
        my -= m_ViewportBounds[0].y;
        auto viewportWidth = m_ViewportBounds[1].x - m_ViewportBounds[0].x;
        auto viewportHeight = m_ViewportBounds[1].y - m_ViewportBounds[0].y;
        my = viewportHeight - my;

        if (mx >= 0 && my >= 0 && mx < viewportWidth && my < viewportHeight)
        {
            int pixelID = m_Viewport_FBO->ReadPixel(1, mx, my);
            m_Entity_Hovered = pixelID == -1 || !m_ActiveScene->EntityExists((entt::entity)pixelID)
                ? Entity() : Entity((entt::entity)pixelID, m_ActiveScene.get());
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
        style.WindowMinSize.x = 300.0f;
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
                if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S", nullptr))
                    SaveSceneAs();

                if (ImGui::MenuItem("Quit")) Application::Get().Close();
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        // --------------------
        // Panels
        // --------------------
        m_SceneHierarchyPanel.OnImGuiRender();
        m_ContentBrowserPanel.OnImGuiRender();
        m_EnvironmentPanel.OnImGuiRender();
        m_TexturePreviewPanel.OnImGuiRender();

        // --------------------
        // Settings panel
        // --------------------
        ImGui::Begin("Settings");
        ImGui::Text("Renderer stats:");
        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("MaxTriangles: %d", Renderer::GetRendererData().GetMaxTriangles());
        ImGui::Text("DrawCalls: %d", Renderer::GetStats().DrawCalls);
        // ImGui::Text("Quads: %d", Renderer2D::GetStats().QuadCount);
        // ImGui::Text("Circles: %d", Renderer2D::GetStats().CircleCount);
        // ImGui::Text("Lines: %d", Renderer2D::GetStats().LineCount);
        ImGui::Text("Triangles: %d", Renderer::GetStats().GetTotalTrianglesCount());
        ImGui::Text("Vertices: %d", Renderer::GetStats().GetTotalVerticesCount());

        std::string entityName = "Null";
        if (m_Entity_Hovered)
            entityName = m_Entity_Hovered.GetComponent<TagComponent>().Tag;
        ImGui::Text("EntityHoverd: %s", entityName.c_str());
        ImGui::Separator();

        if(ImGui::Button("ShowSkyboxTexture"))
        {
            TexturePreviewPanel::Open();
            Ref<Texture2D> texture = m_ViewportRenderer->GetSkyboxPass()->GetOutput(0);
            TexturePreviewPanel::SetTexture(texture);
        }

        std::string buttons[8] = {
            "Position", "Normal", "Depth", "Ambient", "Diffuse", "Specular", "Metalness", "Roughness"
        };

        for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++)
        {
            std::string buttonLabel = "ShowGeometryTexture_" + buttons[i];
            if(ImGui::Button(buttonLabel.c_str()))
            {
                TexturePreviewPanel::Open();
                Ref<Texture2D> texture = m_ViewportRenderer->GetGeometryPass()->GetOutput(i);
                TexturePreviewPanel::SetTexture(texture);
            }
        }

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
    
        // uint32_t textureID = m_Viewport_FBO->GetColorAttachmentRendererID(0);
        uint32_t textureID = m_ViewportRenderer->GetCompositePass()->GetOutput(0)->GetRendererID();
        ImGui::Image((ImTextureID)(uintptr_t)textureID, m_ViewportSize, ImVec2(0, 1), ImVec2(1, 0));

        // Viewport bounds
        auto windowSize = ImGui::GetWindowSize();
        ImVec2 minBound = ImGui::GetWindowPos();
        minBound.x += viewportOffset.x;
        minBound.y += viewportOffset.y;

        ImVec2 maxBound = { minBound.x + windowSize.x, minBound.y + windowSize.y };
        m_ViewportBounds[0] = { minBound.x, minBound.y };
        m_ViewportBounds[1] = { maxBound.x, maxBound.y };
        m_AllowViewportCameraEvents = ImGui::IsMouseHoveringRect(minBound, maxBound);

        // Drag and drop
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                const wchar_t* path = (const wchar_t*)payload->Data;
                CZ_INFO("Drop target: {0}", (char*)path);
                std::filesystem::path filePath = std::filesystem::path((char*)path);
                std::string fileExtension = filePath.extension().string();
                if (std::regex_match(fileExtension, imagePattern))
                {
                    if (m_Entity_Hovered)
                    {
                        std::filesystem::path texturePath = g_AssetsPath / std::filesystem::path((char*)path);
                        m_Entity_Hovered.GetComponent<SpriteRendererComponent>().Texture = Texture2D::Create(texturePath.string());
                    }
                }
                else
                    OpenScene(g_AssetsPath / std::filesystem::path((char*)path));
            }

            ImGui::EndDragDropTarget();
        }

        // Gizmos
        Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
        if (selectedEntity)
        {
            bool isExcluded = (selectedEntity.HasComponent<CameraComponent>() && selectedEntity.GetComponent<CameraComponent>().Primary);
            isExcluded = (isExcluded && m_SceneState != SceneState::Edit) || selectedEntity.HasComponent<SkyLightComponent>();

            if (!isExcluded)
            {
                ImGuizmo::SetOrthographic(false);
                ImGuizmo::SetDrawlist();

                float windowWidth = ImGui::GetWindowWidth();
                float windowHeight = ImGui::GetWindowHeight();
                ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

                // Camera;
                glm::mat4 cameraProjection, cameraView;
                switch (m_SceneState)
                {
                    case SceneState::Edit:
                    {
                        const auto& camera = m_EditorCamera;
                        cameraProjection = camera.GetProjection();
                        cameraView = camera.GetViewMatrix();
                        break;
                    }
                    case SceneState::Play:
                    {
                        auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
                        const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
                        cameraProjection = camera.GetProjection();
                        cameraView = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());
                        break;
                    }
                }

                // Entity transform
                auto& tc = selectedEntity.GetComponent<TransformComponent>();
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
        }

        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::End();

        RenderUI_Toolbar();

        ImGui::End();
    }

    void EditorLayer::RenderUI_Toolbar()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 5));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        auto& colors = ImGui::GetStyle().Colors;
        auto& buttonHoveredColor = colors[ImGuiCol_ButtonHovered];
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHoveredColor.x, buttonHoveredColor.y, buttonHoveredColor.z, 0.5f));
        auto& buttonActivedColor = colors[ImGuiCol_ButtonActive];
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActivedColor.x, buttonActivedColor.y, buttonActivedColor.z, 0.5f));

        ImGui::Begin("Toolbar##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        Ref<Texture2D> icon = m_SceneState == SceneState::Edit ? m_IconPlay : m_IconStop;
        float size = ImGui::GetWindowHeight() - 10.0f;
        ImGui::SameLine(ImGui::GetWindowContentRegionMax().x * 0.5f - size * 0.5f);
        if (ImGui::ImageButton((ImTextureID)(uintptr_t)icon->GetRendererID(), {size, size}, {0, 0}, {1, 1}, 0))
        {
            if (m_SceneState == SceneState::Edit)
                OnScenePlay();
            else if (m_SceneState == SceneState::Play)
                OnSceneStop();
        }
        ImGui::End();

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);
    }

    void EditorLayer::OnEvent(Event &e)
    {
        // m_CameraController->OnEvent(e);
        if (m_SceneState == SceneState::Edit && m_AllowViewportCameraEvents)
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
        if (e.GetMouseButton() == MouseButton::Left && !Input::IsKeyPressed(Key::LeftAlt) && !ImGuizmo::IsUsing() && !ImGuizmo::IsOver() && m_ViewportHovered && m_AllowViewportCameraEvents)
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
        m_ActiveScene->SetFinalPipeline(m_Pipeline);
        m_ActiveScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
        m_ViewportRenderer->SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
        m_ViewportRenderer->SetScene(m_ActiveScene);
        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
        m_EnvironmentPanel.SetContext(m_ActiveScene);
        m_SceneFileName = "";
    }

    void EditorLayer::OpenScene()
    {
        std::string filepath = FileDialogs::OpenFile("Chozo Scene (*.chozo)\0*.chozo\0");
        OpenScene(filepath);
    }

    void EditorLayer::OpenScene(const std::filesystem::path &path)
    {
        if (!path.empty())
        {
            m_ActiveScene = std::make_shared<Scene>();
            m_ActiveScene->SetFinalPipeline(m_Pipeline);
            m_ActiveScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
            m_ViewportRenderer->SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
            m_ViewportRenderer->SetScene(m_ActiveScene);
            m_SceneHierarchyPanel.SetContext(m_ActiveScene);
            m_EnvironmentPanel.SetContext(m_ActiveScene);

            SceneSerializer serializer(m_ActiveScene);
            serializer.Deserialize(path.string());
        }

        m_SceneFileName = path.filename();
    }

    void EditorLayer::SaveSceneAs()
    {
        std::string filepath = FileDialogs::SaveFile("Chozo Scene (*.chozo)\0*.chozo\0", m_SceneFileName);
        if (!filepath.empty())
        {
            SceneSerializer serializer(m_ActiveScene);
            serializer.Serialize(filepath);
        }
    }

    void EditorLayer::OnScenePlay()
    {
        if (m_ActiveScene->GetPrimaryCameraEntity())
            m_SceneState = SceneState::Play;
        else
            CZ_WARN("No camera actived!");
    }

    void EditorLayer::OnSceneStop()
    {
        m_SceneState = SceneState::Edit;
    }
}
