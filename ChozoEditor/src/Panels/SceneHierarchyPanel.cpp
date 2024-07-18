#include "SceneHierarchyPanel.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui_internal.h>

namespace Chozo {
    SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene> context)
    {
        SetContext(context);
    }

    void SceneHierarchyPanel::SetContext(const Ref<Scene> context)
    {
        m_Context = context;
        m_SelectionContext = {};
    }

    void SceneHierarchyPanel::OnImGuiRender()
    {
        ImGui::Begin("Scene Hierarchy");
        m_Context->m_Registry.view<TagComponent>().each([&](auto entity, TagComponent& tc) {
            Entity ent{ entity, m_Context.get() };
            DrawEntityNode(ent);
        });

        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
        {
            m_SelectionContext = {};
        }

        // Right-click on blank space
        if (ImGui::BeginPopupContextWindow(0, 1 | ImGuiPopupFlags_NoOpenOverItems))
        {
            if (ImGui::MenuItem("Create Empty Entity"))
                m_Context->CreateEntity("Empty Entity");

            ImGui::EndPopup();
        }
        ImGui::End();

        ImGui::Begin("Properties");
        if (m_SelectionContext)
        {
            DrawComponents(m_SelectionContext);
        }
        ImGui::End();
    }

    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        bool entityDeleted = false;

        auto& tag = entity.GetCompoent<TagComponent>().Tag;

        ImGuiTreeNodeFlags flags = (m_SelectionContext == entity ? ImGuiTreeNodeFlags_Selected : 0)
            | ImGuiTreeNodeFlags_OpenOnArrow;
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
        
        bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", tag.c_str());

        if (ImGui::IsItemClicked())
        {
            m_SelectionContext = entity;
        }

        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Delete Entity"))
                entityDeleted = true;
            ImGui::EndPopup();
        }

        if (opened)
        {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
            bool opened = ImGui::TreeNodeEx((void*)1909374, flags, "%s", tag.c_str());
            ImGui::TreePop();
        }

        if (entityDeleted)
        {
            m_Context->DestroyEntity(entity);
            if (m_SelectionContext == entity)
                m_SelectionContext = {};
        }
    }

    static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float valueSpeed = 0.1f, float columnWidth = 100.0f)
    {
        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];

        ImGui::PushID(label.c_str());

        ImGui::Columns(2, 0, false);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text("%s", label.c_str());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.5f, 0.1f, 0.15f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.5f, 0.1f, 0.15f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("X", buttonSize))
            values.x = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##X", &values.x, valueSpeed, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.5f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.5f, 0.2f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Y", buttonSize))
            values.y = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Y", &values.y, valueSpeed, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.5f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.5f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Z", buttonSize))
            values.z = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Z", &values.z, valueSpeed, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PopStyleVar();

        ImGui::Columns(1);

        ImGui::PopID();
    }

    template<typename targetValueType, typename UIFunction>
    static void DrawColumnValue(const std::string& name, targetValueType& target, UIFunction uiFunction)
    {
        ImGui::Columns(2, 0, false);
        ImGui::SetColumnWidth(0, 100.0f);
        ImGui::Text("%s", name.c_str());
        ImGui::NextColumn();
        uiFunction(target);
        ImGui::Columns(1);
    }

    template<typename T, typename UIFunction>
    static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
    {
        const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen
            | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth
            | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding;
        if (entity.HasComponent<T>())
        {
            auto& component = entity.GetCompoent<T>();
            ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

            float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
            ImGui::Separator();
            bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, "%s", name.c_str());

            ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
            if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
            {
                ImGui::OpenPopup("ComponentSetting");
            }

            bool removeComponent = false;
            if (ImGui::BeginPopup("ComponentSetting"))
            {
                if (ImGui::MenuItem("Remove component"))
                    removeComponent = true;
                
                ImGui::EndPopup();
            }

            if (open)
            {
                uiFunction(component);
                ImGui::TreePop();
            }

            if (removeComponent)
                entity.RemoveCompoent<T>();
        }
    }

    void SceneHierarchyPanel::DrawComponents(Entity entity)
    {
        if (entity.HasComponent<TagComponent>())
        {
            auto& tag = entity.GetCompoent<TagComponent>().Tag;

            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strcpy(buffer, tag.c_str());

            if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
            {
                tag = std::string(buffer);
            }
        }

        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        if (ImGui::Button("Add Component"))
            ImGui::OpenPopup("AddComponent");

        if (ImGui::BeginPopup("AddComponent"))
        {
            if (ImGui::MenuItem("Camera"))
            {
                if (!m_SelectionContext.HasComponent<CameraComponent>())
                    m_SelectionContext.AddCompoent<CameraComponent>();
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Sprite Renderer"))
            {
                if (!m_SelectionContext.HasComponent<SpriteRendererComponent>())
                    m_SelectionContext.AddCompoent<SpriteRendererComponent>(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        DrawComponent<TransformComponent>("Transform", entity, [](auto& compoent)
        {
            DrawVec3Control("Translation", compoent.Translation);
            glm::vec3 rotation = glm::degrees(compoent.Rotation);
            DrawVec3Control("Rotation", rotation, 0.0f, 1.0f);
            compoent.Rotation = glm::radians(rotation);
            DrawVec3Control("Scale", compoent.Scale, 1.0f);
        });

        DrawComponent<CameraComponent>("Camera", entity, [&](auto& compoent)
        {
            auto& camera = compoent.Camera;

            DrawColumnValue<bool>("Primary", compoent.Primary, [&](auto& target) {
                ImGui::Checkbox("##Primary", &target);
            });
            DrawColumnValue<bool>("Fixed Aspect Ratio", compoent.FixedAspectRatio, [&](auto& target) {
                bool isFixed = ImGui::Checkbox("##Fixed Aspect Ratio", &target);
                if (isFixed)
                    m_Context->OnViewportResize(m_Context->m_ViewportWidth, m_Context->m_ViewportHeight);
            });

            // Camera types
            const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
            const char* currentProjectionType = projectionTypeStrings[(int)camera.GetProjectionType()];
            DrawColumnValue<const char*>("Projection", currentProjectionType, [&](auto& target) {
                if (ImGui::BeginCombo("##Projection", target))
                {
                    for (int i = 0; i < 2; i++)
                    {
                        bool isSelected = target == projectionTypeStrings[i];
                        if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
                        {
                            target = projectionTypeStrings[i];
                            camera.SetProjectionType((SceneCamera::ProjectionType)i);
                        }
                        
                        if (isSelected)
                            ImGui::SetItemDefaultFocus();
                    }

                    ImGui::EndCombo();
                }
            });

            if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
            {
                float verticalFOV = glm::degrees(camera.GetPerspectiveVerticalFOV());
                DrawColumnValue<float>("Vertical FOV", verticalFOV, [&](auto& target) {
                    if (ImGui::DragFloat("##Vertical FOV", &target))
                        camera.SetPerspectiveVerticalFOV(glm::radians(target));
                });

                float near = camera.GetPerspectiveNearClip();
                DrawColumnValue<float>("Near", near, [&](auto& target) {
                    if (ImGui::DragFloat("##Near", &target))
                        camera.SetPerspectiveNearClip(target);
                });

                float far = camera.GetPerspectiveFarClip();
                DrawColumnValue<float>("Far", far, [&](auto& target) {
                    if (ImGui::DragFloat("##Far", &target))
                        camera.SetPerspectiveFarClip(target);
                });
            }

            if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
            {
                float orthoSize = camera.GetOrthographicSize();
                DrawColumnValue<float>("Size", orthoSize, [&](auto& target) {
                    if (ImGui::DragFloat("##Size", &target))
                        camera.SetOrthographicSize(target);
                });

                float near = camera.GetOrthographicNearClip();
                DrawColumnValue<float>("Near", near, [&](auto& target) {
                    if (ImGui::DragFloat("##Near", &target))
                        camera.SetOrthographicNearClip(target);
                });

                float far = camera.GetOrthographicFarClip();
                DrawColumnValue<float>("Far", far, [&](auto& target) {
                    if (ImGui::DragFloat("##Far", &target))
                        camera.SetOrthographicFarClip(target);
                });
            }
        });

        DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component)
        {
            DrawColumnValue<glm::vec4>("Color", component.Color, [&](auto& target) {
                ImGui::ColorEdit4("##Color", glm::value_ptr(target));
            });
        });
    }
}