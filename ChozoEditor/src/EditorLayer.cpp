#include "EditorLayer.h"
#include "CameraController.h"

#include <regex>

#include "Panels/TextureViewerPanel.h"

#include "Chozo/Utilities/PlatformUtils.h"
#include "Chozo/ImGui/ImGuiUI.h"
#include "Chozo/Math/Math.h"

// TODO: Remove
#include "Thumbnail/ThumbnailRenderer.h"
#include "Thumbnail/ThumbnailManager.h"

namespace Chozo {

    std::string ReadFile(const std::string &filepath)
    {
        std::string result;
        if (std::ifstream in(filepath, std::ios::in | std::ios::binary); in)
        {
            in.seekg(0, std::ios::end);
            result.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(&result[0], static_cast<std::streamsize>(result.size()));
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
        m_IconPlay = Texture2D::Create(std::string("../resources/icons/Toolbar/play.png"));
        m_IconStop = Texture2D::Create(std::string("../resources/icons/Toolbar/stop.png"));
        // --------------------
        // Scene
        // --------------------
        m_ActiveScene = Ref<Scene>::Create();
		m_ViewportRenderer = Ref<SceneRenderer>::Create(m_ActiveScene);
        m_ViewportRenderer->SetActive(true);
        // --------------------
        // Editor Camera
        // --------------------
        m_EditorCamera = EditorCamera(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
        // --------------------
        // Panels
        // --------------------
        SceneHierarchyPanel::SetContext(m_ActiveScene);
        PropertiesPanel::SetContext(m_ActiveScene);
        EnvironmentPanel::SetContext(m_ActiveScene);

        ThumbnailRenderer::Init();
        ThumbnailManager::Init();
    }

    void EditorLayer::OnDetach()
    {
        ThumbnailRenderer::Shutdown();
        ThumbnailManager::Shutdown();
    }

    void EditorLayer::OnUpdate(TimeStep ts)
    {
        // Resize
        m_ActiveScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
        m_ViewportRenderer->SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
        m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);

        // Update scene
        switch (m_SceneState)
        {
            case SceneState::Edit:
            {
                if (m_AllowViewportCameraEvents)
                    m_EditorCamera.OnUpdate(ts);

                m_ActiveScene->OnUpdateEditor(ts);
                m_ActiveScene->OnRenderEditor(m_ViewportRenderer, ts, m_EditorCamera);
                break;
            }
            case SceneState::Play:
            {
                m_ActiveScene->OnUpdateRuntime(ts);
                break;
            }
        }
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
                    NewProject();
                if (ImGui::MenuItem("Open...", "Ctrl+O"))
                    OpenProject();
                if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S", nullptr))
                    SaveProjectAs();

                if (ImGui::MenuItem("Quit")) Application::Get().Close();
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Renderer"))
            {
                if (ImGui::MenuItem("Recompile Shaders"))
                    Renderer::GetShaderLibrary()->Recompile();

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        // --------------------
        // Panels
        // --------------------
        m_SceneHierarchyPanel.OnImGuiRender();
        m_PropertiesPanel.OnImGuiRender();
        m_ContentBrowserPanel.OnImGuiRender();
        m_EnvironmentPanel.OnImGuiRender();
        m_MaterialPanel.OnImGuiRender();
        m_TextureViewerPanel.OnImGuiRender();

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
        ImGui::Text("ClearColor:"); ImGui::SameLine();
        ImGui::ColorEdit4("##ClearColor", glm::value_ptr(Renderer::GetConfig().ClearColor));

        std::string entityName = "Null";
        if (m_Entity_Selected)
            entityName = m_Entity_Selected.GetComponent<TagComponent>().Tag;
        ImGui::Text("EntityHoverd: %s", entityName.c_str());
        ImGui::Separator();

        if(ImGui::Button("ShowSkyboxTexture"))
        {
            TextureViewerPanel::Open();
            Ref<Texture2D> texture = m_ViewportRenderer->GetSkyboxPass()->GetOutput(0);
            TextureViewerPanel::SetTexture(texture);
        }

        std::string buttons[4] = {
            "Position", "Normal", "Depth", "Albedo"
        };
        for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++)
        {
            std::string buttonLabel = buttons[i];
            if(ImGui::Button(buttonLabel.c_str()))
            {
                TextureViewerPanel::Open();
                Ref<Texture2D> texture = m_ViewportRenderer->GetGeometryPass()->GetOutput(i);
                TextureViewerPanel::SetTexture(texture);
            }
        }

        std::string materialButtons[4] = {
             "Ambient", "Specular", "Metalness", "Roughness"
        };

        {
            std::string buttonLabel = "ID";
            if(ImGui::Button(buttonLabel.c_str()))
            {
                TextureViewerPanel::Open();
                Ref<Texture2D> texture = m_ViewportRenderer->GetIDPass()->GetOutput(0);
                TextureViewerPanel::SetTexture(texture);
            }
        }

        {
            std::string buttonLabel = "Irradiance";
            if(ImGui::Button(buttonLabel.c_str()))
            {
                TextureViewerPanel::Open();
                Ref<TextureCube> texture = Renderer::GetIrradianceTextureCube();
                TextureViewerPanel::SetTexture(texture);
            }
        }

        {
            std::string buttonLabel = "Prefiltered";
            if(ImGui::Button(buttonLabel.c_str()))
            {
                TextureViewerPanel::Open();
                Ref<TextureCube> texture = Renderer::GetPrefilteredTextureCube();
                TextureViewerPanel::SetTexture(texture);
            }
        }

        {
            std::string buttonLabel = "BRDFLut";
            if(ImGui::Button(buttonLabel.c_str()))
            {
                TextureViewerPanel::Open();
                Ref<Texture2D> texture = Renderer::GetBRDFLutTexture();
                TextureViewerPanel::SetTexture(texture);
            }
        }

        {
            std::string buttonLabel = "Thumbnail";
            if(ImGui::Button(buttonLabel.c_str()))
            {
                TextureViewerPanel::Open();

                ThumbnailRenderer::GetRenderer<MaterialThumbnailRenderer>()->Update();
                Ref<Texture2D> texture = ThumbnailRenderer::GetRenderer<MaterialThumbnailRenderer>()->GetOutput();
                TextureViewerPanel::SetTexture(texture);
            }
        }

        {
            std::string buttonLabel = "Solid";
            if(ImGui::Button(buttonLabel.c_str()))
            {
                TextureViewerPanel::Open();

                Ref<Texture2D> texture = m_ViewportRenderer->GetSolidPass()->GetOutput(0);
                TextureViewerPanel::SetTexture(texture);
            }
        }

        {
            std::string buttonLabel = "Solid Depth";
            if(ImGui::Button(buttonLabel.c_str()))
            {
                TextureViewerPanel::Open();

                Ref<Texture2D> texture = m_ViewportRenderer->GetSolidPass()->GetOutput(1);
                TextureViewerPanel::SetTexture(texture);
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
        UI::BeginDragAndDrop([this](AssetHandle handle) {
            OnDragAndDrop(handle);
        });

        // Gizmos
        Entity selectedEntity;

        if (PropertiesPanel::GetContext() == m_ActiveScene)
            selectedEntity = PropertiesPanel::GetSelectedEntity();

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

                bool disabled = Input::IsKeyPressed(CZ_KEY_LEFT_ALT);
                if (ImGuizmo::IsUsing() && !disabled)
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
        dispatcher.Dispatch<MouseButtonReleasedEvent>(CZ_BIND_EVENT_FN(EditorLayer::OnMouseButtonReleased));
    }

    Entity EditorLayer::PickEntity(float mx, float my)
    {
        Entity entity;

        mx -= m_ViewportBounds[0].x;
        my -= m_ViewportBounds[0].y;
        const auto viewportWidth = m_ViewportBounds[1].x - m_ViewportBounds[0].x;
        const auto viewportHeight = m_ViewportBounds[1].y - m_ViewportBounds[0].y;
        my = viewportHeight - my;

        if (mx > 0 && my > 0 && mx < viewportWidth && my < viewportHeight)
        {
            int pixelID = m_ViewportRenderer->GetIDPass()->GetTargetFramebuffer()->ReadPixel(1, static_cast<int>(mx), static_cast<int>(my));
            entity = pixelID == -1 || !m_ActiveScene->EntityExists(static_cast<entt::entity>(pixelID))
                ? Entity() : Entity(static_cast<entt::entity>(pixelID), m_ActiveScene.get());
        }

        return entity;
    }

    void EditorLayer::OnDragAndDrop(AssetHandle handle)
    {
        switch (Ref<Asset> asset =  Application::GetAssetManager()->GetAsset(handle); asset->GetAssetType())
        {
            case AssetType::Material:
            {
                auto [mx, my] = ImGui::GetMousePos();
                auto entity = PickEntity(mx, my);
                if (entity && entity.HasComponent<MeshComponent>())
                {
                    auto tag = entity.GetComponent<TagComponent>().Tag;
                    CZ_CORE_INFO("EditorLayer::OnDragAndDrop-> tag: {}", tag);
                    entity.GetComponent<MeshComponent>().SetMaterial(handle);
                }
                break;
            }
            case AssetType::Scene:
            {
                m_Entity_Selected = Entity();
                m_ActiveScene = asset.As<Scene>();
                m_ActiveScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
                m_ViewportRenderer->SetScene(m_ActiveScene);
                m_ViewportRenderer->SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
                m_SceneFileName = "path.filename()";
                m_EditorCamera.Reset();
                SceneHierarchyPanel::SetContext(m_ActiveScene);
                PropertiesPanel::SetContext(m_ActiveScene);
                EnvironmentPanel::SetContext(m_ActiveScene);
                break;
            }
            case AssetType::MeshSource:
            {
                auto meshSouce = asset.As<MeshSource>();
                const auto mesh = Ref<Mesh>::Create(meshSouce);
                const Entity rootEntity = m_ActiveScene->InstantiateMesh(mesh);
                m_Entity_Selected = rootEntity;
                break;
            }
            default:
                break;
        }
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
                    NewProject();
                break;
            }
            case Key::O:
            {
                if (control)
                    OpenProject();
                break;
            }
            case Key::S:
            {
                if (control && shift)
                    SaveProjectAs();
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
        return false;
    }

    bool EditorLayer::OnMouseButtonReleased(MouseButtonReleasedEvent &e)
    {
        if (e.GetMouseButton() == MouseButton::Left && !Input::IsKeyPressed(Key::LeftAlt) && !ImGuizmo::IsUsing() && !ImGuizmo::IsOver() && m_ViewportHovered && m_AllowViewportCameraEvents)
        {
            auto [mx, my] = ImGui::GetMousePos();
            m_Entity_Selected = PickEntity(mx, my);
            m_SceneHierarchyPanel.SetSelectedEntity(m_Entity_Selected);
            return true;
        }

        return false;
    }

    std::pair<float, float> EditorLayer::GetMouseViewportSpace() const
    {
        auto [mx, my] = ImGui::GetMousePos();
        mx -= m_ViewportBounds[0].x;
        my -= m_ViewportBounds[0].y;
        const auto viewportWidth = m_ViewportBounds[1].x - m_ViewportBounds[0].x;
		const auto viewportHeight = m_ViewportBounds[1].y - m_ViewportBounds[0].y;

		return { (mx / viewportWidth) * 2.0f - 1.0f, ((my / viewportHeight) * 2.0f - 1.0f) * -1.0f };
    }

    void EditorLayer::NewProject()
    {
        OpenProject("");
    }

    void EditorLayer::OpenProject()
    {
        const std::string filepath = FileDialogs::OpenFile("Chozo Project (*.chozo)\0*.chozo\0");
        OpenProject(filepath);
    }

    void EditorLayer::OpenProject(const fs::path &path)
    {
    }

    void EditorLayer::SaveProjectAs()
    {
        // std::string filepath = FileDialogs::SaveFile("Chozo Project (*.chozo)\0*.chozo\0", m_ProjectFileName);
        // if (!filepath.empty())
        // {
        //     ProjectSerializer serializer(m_ActiveProject);
        //     serializer.Serialize(filepath);
        // }
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
