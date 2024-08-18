#include "SceneHierarchyPanel.h"

#include  "Chozo/Renderer/Geometry/Geometry.h"
#include  "Chozo/Renderer/Geometry/BoxGeometry.h"
#include  "Chozo/Renderer/Geometry/SphereGeometry.h"

#include "PropertyUI.h"

#include <glm/gtc/type_ptr.hpp>

namespace Chozo {

    extern const std::filesystem::path g_AssetsPath;

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

        auto& tag = entity.GetComponent<TagComponent>().Tag;

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

    template<typename T, typename UIFunction>
    static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
    {
        const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen
            | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_SpanAvailWidth
            | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding;
        if (entity.HasComponent<T>())
        {
            auto& component = entity.GetComponent<T>();
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
            {
                entity.RemoveComponent<T>();
            }
        }
    }

    void SceneHierarchyPanel::DrawComponents(Entity entity)
    {
        if (entity.HasComponent<TagComponent>())
        {
            auto& tag = entity.GetComponent<TagComponent>().Tag;

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
                    m_SelectionContext.AddComponent<CameraComponent>();
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Sprite Renderer"))
            {
                if (!m_SelectionContext.HasComponent<SpriteRendererComponent>())
                    m_SelectionContext.AddComponent<SpriteRendererComponent>();
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Circle Renderer"))
            {
                if (!m_SelectionContext.HasComponent<CircleRendererComponent>())
                    m_SelectionContext.AddComponent<CircleRendererComponent>();
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::BeginMenu("Mesh"))
            {
                if (ImGui::MenuItem("Assets"))
                {
                    // if (!m_SelectionContext.HasComponent<MeshComponent>())
                    //         m_SelectionContext.AddComponent<MeshComponent>();
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
                        if (!m_SelectionContext.HasComponent<MeshComponent>())
                        {
                            Ref<Geometry> geom = Geometry::Create(GeometryType::Box);
                            geom->SetEntityID((uint32_t)m_SelectionContext);
                            m_SelectionContext.AddComponent<MeshComponent>(geom);
                        }
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("Sphere"))
                    {
                        if (!m_SelectionContext.HasComponent<MeshComponent>())
                        {
                            Ref<Geometry> geom = Geometry::Create(GeometryType::Sphere);
                            geom->SetEntityID((uint32_t)m_SelectionContext);
                            m_SelectionContext.AddComponent<MeshComponent>(geom);
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

            if (ImGui::MenuItem("SkyLight"))
            {
                if (!m_SelectionContext.HasComponent<SkyLightComponent>())
                    m_SelectionContext.AddComponent<SkyLightComponent>();
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        DrawComponent<TransformComponent>("Transform", entity, [](auto& compoent)
        {
            DrawColumnValue<glm::vec3>("Translation", compoent.Translation, [&](auto& target) {
                DrawVec3Control("Translation", compoent.Translation);
            });

            glm::vec3 rotation = glm::degrees(compoent.Rotation);
            DrawColumnValue<glm::vec3>("Rotation", rotation, [&](auto& target) {
                DrawVec3Control("Rotation", rotation, 0.0f, 1.0f);
            });
            compoent.Rotation = glm::radians(rotation);

            DrawColumnValue<glm::vec3>("Scale", compoent.Scale, [&](auto& target) {
                DrawVec3Control("Scale", compoent.Scale, 1.0f);
            });
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
            // Texture
            ImGui::Button("Texture", ImVec2(100.0f, 0.0f));
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    const wchar_t* path = (const wchar_t*)payload->Data;
                    std::filesystem::path texturePath = g_AssetsPath / std::filesystem::path((char*)path);
                    component.Texture = Texture2D::Create(texturePath.string());
                }

                ImGui::EndDragDropTarget();
            }
            DrawColumnValue<float>("Tiling Factor", component.TilingFactor, [&](auto& target) {
                ImGui::DragFloat("##Tiling Factor", &target, 0.1f, 0.0f, 100.0f);
            });
        });

        DrawComponent<CircleRendererComponent>("Circle Renderer", entity, [](auto& component)
        {
            DrawColumnValue<glm::vec4>("Color", component.Color, [&](auto& target) {
                ImGui::ColorEdit4("##Color", glm::value_ptr(target));
            });
            DrawColumnValue<float>("Thickness", component.Thickness, [&](auto& target) {
                ImGui::DragFloat("##Thickness", &target, 0.025f, 0.0f, 1.0f);
            });
            DrawColumnValue<float>("Fade", component.Fade, [&](auto& target) {
                ImGui::DragFloat("##Fade", &target, 0.00025f, 0.0f, 1.0f);
            });
        });

        DrawComponent<MeshComponent>("Mesh", entity, [](auto& component)
        {
            if (Geometry* geom = dynamic_cast<Geometry*>(component.Source.get()))
            {
                if (BoxGeometry* box = dynamic_cast<BoxGeometry*>(geom))
                {
                    float width = box->GetWidth();
                    DrawColumnValue<float>("Width", width, [&](auto& target) {
                        if (ImGui::DragFloat("##Width", &target, 0.1f, 0.0f, 10.0f))
                            box->SetWidth(target);
                    });
                    float height = box->GetHeight();
                    DrawColumnValue<float>("Height", height, [&](auto& target) {
                        if (ImGui::DragFloat("##Height", &target, 0.1f, 0.0f, 10.0f))
                            box->SetHeight(target);
                    });
                    float depth = box->GetDepth();
                    DrawColumnValue<float>("Depth", depth, [&](auto& target) {
                        if (ImGui::DragFloat("##Depth", &target, 0.1f, 0.0f, 10.0f))
                            box->SetDepth(target);
                    });
                    int widthSegments = box->GetWidthSegments();
                    DrawColumnValue<int>("WidthSegments", widthSegments, [&](auto& target) {
                        if (ImGui::DragInt("##WidthSegments", &target, 1, 1, 10))
                            box->SetWidthSegments((uint32_t&)target);
                    });
                    int heightSegments = box->GetHeightSegments();
                    DrawColumnValue<int>("HeightSegments", heightSegments, [&](auto& target) {
                        if (ImGui::DragInt("##HeightSegments", &target, 1, 1, 10))
                            box->SetHeightSegments((uint32_t&)target);
                    });
                    int depthSegments = box->GetDepthSegments();
                    DrawColumnValue<int>("DepthSegments", depthSegments, [&](auto& target) {
                        if (ImGui::DragInt("##DepthSegments", &target, 1, 1, 10))
                            box->SetDepthSegments((uint32_t&)target);
                    });
                }
                if (SphereGeometry* sphere = dynamic_cast<SphereGeometry*>(geom))
                {
                    float m_Radius, m_PhiStart, m_PhiLength, m_ThetaStart, m_ThetaLength;
                    uint32_t m_WidthSegments, m_HeightSegments;

                    float radius = sphere->GetRadius();
                    DrawColumnValue<float>("Radius", radius, [&](auto& target) {
                        if (ImGui::DragFloat("##Radius", &target, 0.1f, 0.0f, 10.0f))
                            sphere->SetRadius(target);
                    });
                    int widthSegments = sphere->GetWidthSegments();
                    DrawColumnValue<int>("WidthSegments", widthSegments, [&](auto& target) {
                        if (ImGui::DragInt("##WidthSegments", &target, 1, 1, 64))
                            sphere->SetWidthSegments((uint32_t&)target);
                    });
                    int heightSegments = sphere->GetHeightSegments();
                    DrawColumnValue<int>("HeightSegments", heightSegments, [&](auto& target) {
                        if (ImGui::DragInt("##HeightSegments", &target, 1, 1, 32))
                            sphere->SetHeightSegments((uint32_t&)target);
                    });
                    float phiStart = sphere->GetPhiStart();
                    DrawColumnValue<float>("PhiStart", phiStart, [&](auto& target) {
                        if (ImGui::DragFloat("##PhiStart", &target, 0.1f, 0.0f, 10.0f))
                            sphere->SetPhiStart(target);
                    });
                    float phiLength = sphere->GetPhiLength();
                    DrawColumnValue<float>("PhiLength", phiLength, [&](auto& target) {
                        if (ImGui::DragFloat("##PhiLength", &target, 0.1f, 0.0f, Math::PI * 2))
                            sphere->SetPhiLength(target);
                    });
                    float thetaStart = sphere->GetThetaStart();
                    DrawColumnValue<float>("ThetaStart", thetaStart, [&](auto& target) {
                        if (ImGui::DragFloat("##ThetaStart", &target, 0.1f, 0.0f, 10.0f))
                            sphere->SetThetaStart(target);
                    });
                    float thetaLength = sphere->GetThetaLength();
                    DrawColumnValue<float>("ThetaLength", thetaLength, [&](auto& target) {
                        if (ImGui::DragFloat("##ThetaLength", &target, 0.1f, 0.0f, Math::PI))
                            sphere->SetThetaLength(target);
                    });
                }
            }
        });

        DrawComponent<SkyLightComponent>("SkyLight", entity, [](auto& component)
        {
            DrawColumnValue<float>("Skybox LOD", component.Lod, [&](auto& target) {
                ImGui::DragFloat("##Skybox LOD", &target, 0.1f, 0.0f, 10.0f);
            });

            DrawColumnValue<float>("Intensity", component.Intensity, [&](auto& target) {
                ImGui::DragFloat("##Intensity", &target, 0.1f, 0.0f, 10.0f);
            });

            DrawColumnValue<glm::vec3>("TurbidityAzimuthInclination", component.TurbidityAzimuthInclination, [&](auto& target) {
                if (DrawVec3Control("TurbidityAzimuthInclination", component.TurbidityAzimuthInclination, 0.0f, 0.01f))
                    RenderCommand::RenderPreethamSky(component.TurbidityAzimuthInclination.x, component.TurbidityAzimuthInclination.y, component.TurbidityAzimuthInclination.z);
            });

            // DrawColumnValue<glm::vec3>("Light Direction", s_LightDirection, [&](auto& target) {
            //     DrawVec3Control("Light Direction", s_LightDirection, 0.0f, 1.0f);
            // });

            // DrawColumnValue<float>("Light Multiplier", s_LightMultiplier, [&](auto& target) {
            //     if (ImGui::SliderFloat("##Light Multiplier", &target, 0.0f, 10.0f))
            //         s_LightMultiplier = target;
            // });

            // DrawColumnValue<float>("Exposure", s_Exposure, [&](auto& target) {
            //     if (ImGui::SliderFloat("##Exposure", &target, 0.0f, 10.0f))
            //         s_Exposure = target;
            // });

            // DrawColumnValue<bool>("Radiance Prefiltering", s_RadiancePrefiltering, [&](auto& target) {
            //     ImGui::Checkbox("##Radiance Prefiltering", &target);
            // });

            // DrawColumnValue<float>("EnvMapRotation", s_EnvMapRotation, [&](auto& target) {
            //     if (ImGui::DragFloat("##EnvMapRotation", &target, 0.1f, 0.0f, 10.0f))
            //         s_EnvMapRotation = target;
            // });

            // DrawColumnValue<bool>("Show Bounding Boxes", s_ShowBoundingBoxes, [&](auto& target) {
            //     ImGui::Checkbox("##Show Bounding Boxes", &target);
            // });
        });
    }
}