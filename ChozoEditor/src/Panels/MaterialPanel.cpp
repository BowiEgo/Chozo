#include "MaterialPanel.h"

#include "PropertyUI.h"

namespace Chozo {

	MaterialPanel* MaterialPanel::s_Instance;
    bool MaterialPanel::s_Show = false;

    MaterialPanel::MaterialPanel()
    {
        s_Instance = this;
    }
    void MaterialPanel::OnImGuiRender()
    {
        if (!s_Show)
            return;

        ImGui::Begin("Material");

        Ref<Texture2D> preview;
        if (m_PreviewUpdated)
        {
            ThumbnailExporter::GetMaterialThumbnailRenderer()->OnUpdate();
            preview = ThumbnailExporter::GetMaterialThumbnailRenderer()->GetOutput();
            m_PreviewCache = preview;
            m_PreviewUpdated = false;
        }
        else
        {
            preview = m_PreviewCache;
        }

        RenderPreviewImage(preview);

        if (!m_Material)
            m_Material = ThumbnailExporter::GetMaterialThumbnailRenderer()->GetMaterial();

        for (auto& pair : m_Material->GetUniforms())
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
                            m_Material->Set(pair.first, value);
                            OnMaterialChange(m_Material, pair.first, value);
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
                            m_Material->Set(pair.first, value);
                            OnMaterialChange(m_Material, pair.first, value);
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
                        {
                            m_Material->Set(pair.first, value);
                            OnMaterialChange(m_Material, pair.first, value);
                        }
                    });
                }
            }
            else if (std::holds_alternative<float>(pair.second))
            {
                float& value = std::get<float>(pair.second);
                DrawColumnValue<float>(name, value, [&](auto& target) {
                    if (ImGui::DragFloat(("##" + name).c_str(), &target, 0.0025f, 0.0f, 1.0f))
                    {
                        m_Material->Set(pair.first, value);
                        OnMaterialChange(m_Material, pair.first, value);
                    }
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

        ImGui::End();
    }

    void MaterialPanel::RenderPreviewImage(Ref<Texture2D> texture)
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

    void MaterialPanel::OnMaterialChange(Ref<Material> material, std::string name, UniformValue value)
    {
        ThumbnailExporter::GetMaterialThumbnailRenderer()->SetMaterialValue(material, name, value);
    }

    void MaterialPanel::SetThumbnailMaterial(Entity entity)
    {
        if (m_Context != ThumbnailExporter::GetMaterialThumbnailRenderer()->GetScene())
        {
            auto material = entity.GetComponent<MeshComponent>().MaterialInstance;
            ThumbnailExporter::GetMaterialThumbnailRenderer()->SetMaterial(material);
        }
    }

}
