#include "SceneHierarchyPanel.h"

#include  "Chozo/Renderer/Geometry/Geometry.h"
#include  "Chozo/Renderer/Geometry/BoxGeometry.h"
#include  "Chozo/Renderer/Geometry/SphereGeometry.h"
#include "Chozo/ImGui/ImGuiUI.h"

#include "PropertyUI.h"

#include <glm/gtc/type_ptr.hpp>

namespace Chozo {

	SceneHierarchyPanel* SceneHierarchyPanel::s_Instance;

    SceneHierarchyPanel::SceneHierarchyPanel()
    {
        s_Instance = this;
    }

    SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene> context)
    {
        SetContext(context);
    }

    void SceneHierarchyPanel::SetContext(const Ref<Scene> context)
    {
        s_Instance->m_Context = context;
        s_Instance->SetSelectedEntity({});
    }

    void SceneHierarchyPanel::OnImGuiRender()
    {
        ImGui::Begin("Scene Hierarchy");
        m_Context->m_Registry.view<TagComponent>().each([&](auto entity, TagComponent& tc) {
            Entity ent{ entity, m_Context.get() };
            DrawEntityNode(ent);
        });

        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
            SetSelectedEntity({});

        // Right-click on blank space
        if (ImGui::BeginPopupContextWindow(0, 1 | ImGuiPopupFlags_NoOpenOverItems))
        {
            // ImGui::OpenPopup("AddComponent");
            m_SelectionContext = DrawAddComponent();
            ImGui::EndPopup();
        }
        ImGui::End();
    }

    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        bool entityDeleted = false;

        auto& tag = entity.GetComponent<TagComponent>().Tag;

        ImGuiTreeNodeFlags flags = (m_SelectionContext == entity ? ImGuiTreeNodeFlags_Selected : 0)
            | ImGuiTreeNodeFlags_OpenOnArrow;
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 2.0f, 0.0f });
        bool opened = ImGui::TreeNodeEx((void*)(uint64_t)entity, flags, "%s", tag.c_str());
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
            // if (entity.HasComponent<MeshComponent>())
            // {
            //     for (int i = 0; i < std::size(s_MeshLeafs); i++)
            //     {
            //         if (ImGui::Selectable(s_MeshLeafs[i].c_str(), m_SelectionContext == entity))
            //         {
            //             m_SelectionContext = entity;
            //         }
            //     }
            // }
            ImGui::TreePop();
        }

        if (entityDeleted)
        {
            m_Context->DestroyEntity(entity);
            if (m_SelectionContext == entity)
                SetSelectedEntity({});
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
                        auto geom = Ref<BoxGeometry>::Create();
                        geom->SetEntityID((uint64_t)entity);
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
                        auto geom = Ref<SphereGeometry>::Create();
                        geom->SetEntityID((uint64_t)entity);
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

        return entity;
    }
}