#include "PropertiesPanel.h"

#include "SceneHierarchyPanel.h"

#include "Chozo/Renderer/Geometry/Geometry.h"
#include "Chozo/Renderer/Geometry/BoxGeometry.h"
#include "Chozo/Renderer/Geometry/SphereGeometry.h"
#include "Chozo/ImGui/ImGuiUI.h"
#include "Chozo/Thumbnail/ThumbnailExporter.h"

#include "PropertyUI.h"

#include <glm/gtc/type_ptr.hpp>

namespace Chozo {

	PropertiesPanel* PropertiesPanel::s_Instance;

    PropertiesPanel::PropertiesPanel()
    {
        s_Instance = this;
    }

    PropertiesPanel::PropertiesPanel(const Ref<Scene> context)
    {
        SetContext(context);
    }

    bool PropertiesPanel::Filtered(PropertyType type)
    {
        auto it = std::find(m_Filter.begin(), m_Filter.end(), type);
        return it != m_Filter.end();
    }

    void PropertiesPanel::SetContext(const Ref<Scene> context)
    {
        s_Instance->m_Context = context;
        s_Instance->SetSelectedEntity({});
    }

    void PropertiesPanel::OnImGuiRender()
    {
        ImGui::Begin("Properties");
        if (m_SelectionContext)
            DrawComponents(m_SelectionContext);
        ImGui::End();
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
            bool open = ImGui::TreeNodeEx((void*)std::hash<std::string>{}(name), treeNodeFlags, "%s", name.c_str());

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

    void PropertiesPanel::DrawComponents(Entity entity)
    {
        if (Filtered(PropertyType::Tag))
            DrawTagProperties(m_SelectionContext);

        if (Filtered(PropertyType::Transform))
            DrawTransformProperties(m_SelectionContext);

        if (Filtered(PropertyType::Camera))
            DrawCameraProperties(m_SelectionContext);

        if (Filtered(PropertyType::Sprite))
            DrawSpriteProperties(m_SelectionContext);

        if (Filtered(PropertyType::Circle))
            DrawCircleProperties(m_SelectionContext);

        if (Filtered(PropertyType::Mesh))
            DrawMeshProperties(m_SelectionContext);

        if (Filtered(PropertyType::SkyLight))
            DrawSkyLightProperties(m_SelectionContext);

        if (Filtered(PropertyType::Light))
            DrawLightProperties(m_SelectionContext);

        if (Filtered(PropertyType::Geometry))
            DrawGeometryProperties(m_SelectionContext);

        if (Filtered(PropertyType::Material))
            DrawMaterialProperties(m_SelectionContext);
    }

    void PropertiesPanel::DrawTagProperties(Entity entity)
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

            ImGui::SameLine();
            ImGui::PushItemWidth(-1);
            if (ImGui::Button("Add Component"))
                ImGui::OpenPopup("AddComponent");

            if (ImGui::BeginPopup("AddComponent"))
            {
                SceneHierarchyPanel::DrawAddComponent(m_SelectionContext);
                ImGui::EndPopup();
            }
        }
    }

    void PropertiesPanel::DrawTransformProperties(Entity entity)
    {
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
    }

    void PropertiesPanel::DrawCameraProperties(Entity entity)
    {
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
    }

    void PropertiesPanel::DrawSpriteProperties(Entity entity)
    {
        DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component)
        {
            DrawColumnValue<glm::vec4>("Color", component.Color, [&](auto& target) {
                ImGui::ColorEdit4("##Color", glm::value_ptr(target));
            });
            // Texture
            ImGui::Button("Texture", ImVec2(100.0f, 0.0f));
            UI::BeginDragAndDrop([](AssetHandle handle){
                // TODO: Finish
            });
            DrawColumnValue<float>("Tiling Factor", component.TilingFactor, [&](auto& target) {
                ImGui::DragFloat("##Tiling Factor", &target, 0.1f, 0.0f, 100.0f);
            });
        });
    }

    void PropertiesPanel::DrawCircleProperties(Entity entity)
    {
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
    }

    void PropertiesPanel::DrawMeshProperties(Entity entity)
    {
        DrawComponent<MeshComponent>("Mesh", entity, [entity](auto& component)
        {
            // Mesh types
            const char* meshTypeStrings[] = { "Dynamic", "Instanced", "Static" };
            const char* currentMeshType = meshTypeStrings[(int)component.Type];
            DrawColumnValue<const char*>("Type", currentMeshType, [&](auto& target) {
                if (ImGui::BeginCombo("##Type", target))
                {
                    for (int i = 0; i < 3; i++)
                    {
                        bool isSelected = target == meshTypeStrings[i];
                        if (ImGui::Selectable(meshTypeStrings[i], isSelected))
                        {
                            target = meshTypeStrings[i];
                            component.Type = MeshType(i);
                            component.GenerateMeshInstance();
                        }
                        
                        if (isSelected)
                            ImGui::SetItemDefaultFocus();
                    }

                    ImGui::EndCombo();
                }
            });
        });
    }

    void PropertiesPanel::DrawSkyLightProperties(Entity entity)
    {
        DrawComponent<SkyLightComponent>("SkyLight", entity, [](SkyLightComponent& component)
        {
            // Camera types
            const char* skyLightTypeStrings[] = { "Static", "Dynamic" };
            const char* currentSkyLightType = skyLightTypeStrings[(int)component.Dynamic];
            DrawColumnValue<const char*>("SkyLight", currentSkyLightType, [&](auto& target) {
                if (ImGui::BeginCombo("##SkyLightType", target))
                {
                    for (int i = 0; i < 2; i++)
                    {
                        bool isSelected = target == skyLightTypeStrings[i];
                        if (ImGui::Selectable(skyLightTypeStrings[i], isSelected))
                        {
                            target = skyLightTypeStrings[i];
                            bool val = target == "Dynamic";
                            if (component.Dynamic != val)
                            {
                                component.Dynamic = (target == "Dynamic");
                                component.SceneEnvironment = nullptr;
                            }
                        }
                        
                        if (isSelected)
                            ImGui::SetItemDefaultFocus();
                    }

                    ImGui::EndCombo();
                }
            });

            if (!component.Dynamic)
            {
                const AssetMetadata metadata = Application::GetAssetManager()->GetMetadata(component.Source);

                DrawColumnPath("File", metadata, [&](AssetMetadata sourceMetadata) {
                    if (sourceMetadata.Type == AssetType::Texture)
                    {
                        component.Source = sourceMetadata.Handle;
                        component.SceneEnvironment = nullptr;
                    }
                });
            }

            DrawColumnValue<float>("Skybox LOD", component.Lod, [&](auto& target) {
                ImGui::DragFloat("##Skybox LOD", &target, 0.1f, 0.0f, 10.0f);
            });

            DrawColumnValue<float>("Intensity", component.Intensity, [&](auto& target) {
                ImGui::DragFloat("##Intensity", &target, 0.01f, 0.0f, 10.0f);
            });

            DrawColumnValue<float>("Turbidity", component.TurbidityAzimuthInclination.x, [&](auto& target) {
                if (ImGui::DragFloat("##Turbidity", &target, 0.01f, 0.0f, 10.0f))
                    Renderer::UpdatePreethamSky(component.TurbidityAzimuthInclination.x, component.TurbidityAzimuthInclination.y, component.TurbidityAzimuthInclination.z);
            });

            DrawColumnValue<float>("Azimuth", component.TurbidityAzimuthInclination.y, [&](auto& target) {
                if (ImGui::DragFloat("##Azimuth", &target, 0.01f, 0.0f, 10.0f))
                    Renderer::UpdatePreethamSky(component.TurbidityAzimuthInclination.x, component.TurbidityAzimuthInclination.y, component.TurbidityAzimuthInclination.z);
            });

            DrawColumnValue<float>("Inclination", component.TurbidityAzimuthInclination.z, [&](auto& target) {
                if (ImGui::DragFloat("##Inclination", &target, 0.01f, 0.0f, 10.0f))
                    Renderer::UpdatePreethamSky(component.TurbidityAzimuthInclination.x, component.TurbidityAzimuthInclination.y, component.TurbidityAzimuthInclination.z);
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

    void PropertiesPanel::DrawLightProperties(Entity entity)
    {
        DrawComponent<DirectionalLightComponent>("Light", entity, [](auto& component)
        {
            DrawColumnValue<glm::vec3>("Direction", component.Direction, [&](auto& target) {
                if (DrawVec3Control("Direction", target, 0.0f, 1.0f))
                    component.Direction = target;
            });
            DrawColumnValue<glm::vec3>("Color", component.Color, [&](auto& target) {
                ImGui::ColorEdit3("##Color", glm::value_ptr(target));
            });
            DrawColumnValue<float>("Intensity", component.Intensity, [&](auto& target) {
                if (ImGui::SliderFloat("##Intensity", &target, 0.0f, 10.0f))
                    component.Intensity = target;
            });
        });

        DrawComponent<PointLightComponent>("Light", entity, [](auto& component)
        {
            DrawColumnValue<glm::vec3>("Color", component.Color, [&](auto& target) {
                ImGui::ColorEdit3("##Color", glm::value_ptr(target));
            });
            DrawColumnValue<float>("Intensity", component.Intensity, [&](auto& target) {
                if (ImGui::SliderFloat("##Intensity", &target, 0.0f, 10.0f))
                    component.Intensity = target;
            });
        });

        DrawComponent<SpotLightComponent>("Light", entity, [](auto& component)
        {
            DrawColumnValue<float>("Intensity", component.Intensity, [&](auto& target) {
                if (ImGui::SliderFloat("##Intensity", &target, 0.0f, 1.0f))
                    component.Intensity = target;
            });
            DrawColumnValue<glm::vec3>("Direction", component.Direction, [&](auto& target) {
                if (DrawVec3Control("Direction", target, 0.0f, 1.0f))
                    component.Direction = target;
            });
            DrawColumnValue<float>("AngleAttenuation", component.AngleAttenuation, [&](auto& target) {
                if (ImGui::SliderFloat("##AngleAttenuation", &target, 0.0f, 10.0f))
                    component.AngleAttenuation = target;
            });
            DrawColumnValue<glm::vec3>("Color", component.Color, [&](auto& target) {
                ImGui::ColorEdit3("##Color", glm::value_ptr(target));
            });
            DrawColumnValue<float>("Angle", component.Angle, [&](auto& target) {
                if (ImGui::SliderFloat("##Angle", &target, 0.0f, 180.0f))
                    component.Angle = target;
            });
        });
    }

    void PropertiesPanel::DrawGeometryProperties(Entity entity)
    {
        DrawComponent<MeshComponent>("Geometry", entity, [entity](auto& component)
        {
            if (Geometry* geom = dynamic_cast<Geometry*>(component.MeshInstance->GetMeshSource().get()))
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
    }

    void PropertiesPanel::DrawMaterialProperties(Entity entity)
    {
        DrawComponent<MeshComponent>("Material", entity, [entity, this](auto& component)
        {
            Ref<Material> material = component.MaterialInstance;

            Ref<Texture2D> preview = ThumbnailExporter::GetMaterialThumbnailRenderer()->GetOutput();
            RenderPreviewImage(preview);

            for (auto& pair : material->GetUniforms())
            {
                std::string name = pair.first.substr(pair.first.find('.') + 1);
                if (std::holds_alternative<glm::vec4>(pair.second))
                {
                    glm::vec4& value = std::get<glm::vec4>(pair.second);
                    if (pair.first.find("Color") > 0 || pair.first.find("Albedo"))
                    {
                        DrawColumnValue<glm::vec4>(name, value, [&](auto& target) {
                            if (ImGui::ColorEdit4(("##" + name).c_str(), glm::value_ptr(target)))
                            {
                                material->Set(pair.first, value);
                                ThumbnailExporter::GetMaterialThumbnailRenderer()->OnUpdate();
                            }
                        });
                    }
                }
                else if (std::holds_alternative<glm::vec3>(pair.second))
                {
                    glm::vec3& value = std::get<glm::vec3>(pair.second);
                    if (pair.first.find("Color") || pair.first.find("Albedo"))
                    {
                        DrawColumnValue<glm::vec3>(name, value, [&](auto& target) {
                            if (ImGui::ColorEdit3(("##" + name).c_str(), glm::value_ptr(target)))
                            {
                                material->Set(pair.first, value);
                                ThumbnailExporter::GetMaterialThumbnailRenderer()->OnUpdate();
                            }
                        });
                        RenderPreviewImage();

                        // if (pair.first.find("Albedo"))
                        // {
                        //     std::string filePath = material->GetUniromSourcePath("u_AlbedoTex");
                        //     UI::FileButton(&filePath);
                        //     material->Set("u_AlbedoTex", filePath);
                        //     ImGui::SameLine(); ImGui::Text("%s", filePath.c_str());
                        // }
                    }
                    else
                    {
                        DrawColumnValue<glm::vec3>(name, value, [&](auto& target) {
                            if (DrawVec3Control("##" + name, target))
                                material->Set(pair.first, value);
                        });
                    }
                }
                else if (std::holds_alternative<float>(pair.second))
                {
                    float& value = std::get<float>(pair.second);
                    DrawColumnValue<float>(name, value, [&](auto& target) {
                        if (ImGui::DragFloat(("##" + name).c_str(), &target, 0.0025f, 0.0f, 1.0f))
                            material->Set(pair.first, value);
                    });
                    if (name == "Metalness")
                    {
                        RenderPreviewImage();
                    }
                    else if (name == "Roughness")
                    {
                        RenderPreviewImage();
                    }
                }

            }
            DrawColumnValue("Normal", [&]() {
                
                RenderPreviewImage();
            });
        });
    }

    void PropertiesPanel::RenderPreviewImage(Ref<Texture2D> texture)
    {
        texture = texture ? texture : Renderer::GetCheckerboardTexture();

        float imageAspectRatio = static_cast<float>(texture->GetHeight()) / static_cast<float>(texture->GetWidth());
        ImVec2 uv0(0.0f, 1.0f);
        ImVec2 uv1(1.0f, 0.0f);
        ImVec2 size(80, 80);

        if (imageAspectRatio <= 1.0f) {
            float offsetY = (1.0f - 1.0f / imageAspectRatio) / 2.0f;
            uv0.y = 1.0f - offsetY;
            uv1.y = offsetY;
        } else {
            float offsetX = (1.0f - imageAspectRatio) / 2.0f;
            uv0.x = offsetX;
            uv1.x = 1.0f - offsetX;
        }

        UI::ScopedColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::InvisibleButton("##thumbnailButton", size);
        UI::DrawButtonImage(texture, IM_COL32(255, 255, 255, 225), UI::RectExpanded(UI::GetItemRect(), -6.0f, -6.0f), uv0, uv1);
    }

} // namespace Chozo
