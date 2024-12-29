#include "SceneHierarchyPanel.h"

#include  "Chozo/Renderer/Geometry/Geometry.h"
#include  "Chozo/Renderer/Geometry/BoxGeometry.h"
#include  "Chozo/Renderer/Geometry/SphereGeometry.h"
#include "Chozo/ImGui/ImGuiUI.h"

#include "PropertyUI.h"

namespace Chozo {

	SceneHierarchyPanel* SceneHierarchyPanel::s_Instance;

    SceneHierarchyPanel::SceneHierarchyPanel()
    {
        s_Instance = this;
    }

    SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
    {
        SetContext(context);
    }

    void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
    {
        s_Instance->m_Context = context;
        s_Instance->SetSelectedEntity({});
    }

    void SceneHierarchyPanel::OnImGuiRender()
    {
        ImGui::Begin("Scene Hierarchy");
        m_Context->m_Registry.view<TagComponent>().each([&](auto entity, TagComponent& tc) {
            Entity ent{ entity, m_Context.get() };
            if (ent.GetParentUUID() == 0)
                DrawEntityNode(ent);
        });

        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
            SetSelectedEntity({});

        // Right-click on blank space
        if (ImGui::BeginPopupContextWindow(nullptr, 1 | ImGuiPopupFlags_NoOpenOverItems))
        {
            // ImGui::OpenPopup("AddComponent");
            m_SelectedEntity = DrawAddComponent();
            ImGui::EndPopup();
        }
        ImGui::End();
    }

    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        bool entityDeleted = false;

        auto& tag = entity.GetComponent<TagComponent>().Tag;

        const ImGuiTreeNodeFlags flags = (m_SelectedEntity == entity ? ImGuiTreeNodeFlags_Selected : 0)
            | (entity.Children().empty() ? ImGuiTreeNodeFlags_Leaf : 0)
            | ImGuiTreeNodeFlags_OpenOnArrow
            | ImGuiTreeNodeFlags_SpanAvailWidth;

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 2.0f, 0.0f });
        bool opened = ImGui::TreeNodeEx(reinterpret_cast<void *>(static_cast<uint64_t>(entity)), flags, "%s", tag.c_str());
        ImGui::PopStyleVar();

        if (ImGui::IsItemClicked())
            SetSelectedEntity(entity);

        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Delete Entity"))
                entityDeleted = true;

            ImGui::EndPopup();
        }

        if (opened)
        {
            for (auto childID : entity.Children())
            {
                DrawEntityNode(m_Context->GetEntityWithUUID(childID));
            }
            ImGui::TreePop();
        }

        if (entityDeleted)
        {
            if (m_SelectedEntity == entity)
                SetSelectedEntity({});
            m_Context->DestroyEntity(entity);
        }
    }

    Entity SceneHierarchyPanel::DrawAddComponent(Entity entity)
    {
        if (ImGui::MenuItem("Camera"))
        {
            if (!entity)
                entity = s_Instance->m_Context->CreateEntity("Camera");
            if (!entity.HasComponent<CameraComponent>())
                entity.AddComponent<CameraComponent>();
            ImGui::CloseCurrentPopup();
        }

        if (ImGui::MenuItem("Sprite Renderer"))
        {
            if (!entity)
                entity = s_Instance->m_Context->CreateEntity("Sprite Renderer");
            if (!entity.HasComponent<SpriteRendererComponent>())
                entity.AddComponent<SpriteRendererComponent>();
            ImGui::CloseCurrentPopup();
        }

        if (ImGui::MenuItem("Circle Renderer"))
        {
            if (!entity)
                entity = s_Instance->m_Context->CreateEntity("Circle Renderer");
            if (!entity.HasComponent<CircleRendererComponent>())
                entity.AddComponent<CircleRendererComponent>();
            ImGui::CloseCurrentPopup();
        }

        if (ImGui::BeginMenu("Mesh"))
        {
            if (ImGui::MenuItem("Assets"))
            {
                // if (!entity.HasComponent<MeshComponent>())
                //         entity.AddComponent<MeshComponent>();
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::BeginMenu("Geometry"))
            {
                if (ImGui::MenuItem("Plane"))
                {
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Box"))
                {
                    if (!entity)
                        entity = s_Instance->m_Context->CreateEntity("Box");
                    if (!entity.HasComponent<MeshComponent>())
                    {
                        auto geom = Geometry::Create<BoxGeometry>();
                        entity.AddComponent<MeshComponent>(geom);
                    }
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Sphere"))
                {
                    if (!entity)
                        entity = s_Instance->m_Context->CreateEntity("Sphere");
                    if (!entity.HasComponent<MeshComponent>())
                    {
                        auto geom = Geometry::Create<SphereGeometry>();
                        entity.AddComponent<MeshComponent>(geom);
                    }
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Cone"))
                {
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem("Cylinder"))
                {
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Light"))
        {
            if (ImGui::MenuItem("Directional"))
            {
                if (!entity)
                    entity = s_Instance->m_Context->CreateEntity("Directional Light");
                if (!entity.HasComponent<DirectionalLightComponent>())
                    entity.AddComponent<DirectionalLightComponent>();
                
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Point"))
            {
                if (!entity)
                    entity = s_Instance->m_Context->CreateEntity("Point Light");
                if (!entity.HasComponent<PointLightComponent>())
                    entity.AddComponent<PointLightComponent>();

                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Spot"))
            {
                if (!entity)
                    entity = s_Instance->m_Context->CreateEntity("Spot Light");
                if (!entity.HasComponent<SpotLightComponent>())
                    entity.AddComponent<SpotLightComponent>();
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("SkyLight"))
        {
            if (!entity)
                entity = s_Instance->m_Context->CreateEntity("SkyLight");
            if (!entity.HasComponent<SkyLightComponent>())
                entity.AddComponent<SkyLightComponent>();
            ImGui::CloseCurrentPopup();
        }

        s_Instance->SetSelectedEntity(entity);

        return entity;
    }
}