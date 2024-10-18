#include "MaterialPanel.h"

#include "Thumbnail/ThumbnailRenderer.h"

#include "PropertyUI.h"

namespace Chozo {

	MaterialPanel* MaterialPanel::s_Instance;
    bool MaterialPanel::s_Show = false;

    Ref<Texture2D>& MaterialPanel::GetPreviewTextureByType(PreviewType type)
    {
        switch (type) {
            #define GENERATE_CASE(ENUM) case PreviewType::ENUM: return m_##ENUM##Texture;
            FOREACH_PREVIEW_TYPE(GENERATE_CASE)
            #undef GENERATE_CASE
            default: return m_AlbedoTexture;
        }
    }

    void MaterialPanel::UpdatePreviewTextureByType(PreviewType type)
    {
        switch (type) {
            #define GENERATE_CASE(ENUM) case PreviewType::ENUM: { \
                auto checkerboard = Renderer::GetCheckerboardTexture(); \
                if (m_##ENUM##Texture && m_##ENUM##Texture != checkerboard) \
                { \
                    m_Material->Set("u_" #ENUM "Tex", m_##ENUM##Texture); \
                    m_Material->Set("u_Material.enable" #ENUM "Tex", true); \
                    OnMaterialChange(m_Material, "u_" #ENUM "Tex", m_##ENUM##Texture); \
                    OnMaterialChange(m_Material, "u_Material.enable" #ENUM "Tex", true); \
                } \
                else \
                { \
                    m_##ENUM##Texture = checkerboard; \
                    m_Material->Set("u_Material.enable" #ENUM "Tex", false); \
                    OnMaterialChange(m_Material, "u_Material.enable" #ENUM "Tex", false); \
                } \
                break; \
            };
            FOREACH_PREVIEW_TYPE(GENERATE_CASE)
            #undef GENERATE_CASE
            default: return;
        }
    }

    void MaterialPanel::RenderPreviewImageByType(PreviewType type)
    {
        switch (type) {
            #define GENERATE_CASE(ENUM) case PreviewType::ENUM: { \
                UI::DrawButtonImageByRatio(m_##ENUM##Texture); \
                break; \
            };
            FOREACH_PREVIEW_TYPE(GENERATE_CASE)
            #undef GENERATE_CASE
            default: return;
        }
    }

    MaterialPanel::MaterialPanel()
    {
        s_Instance = this;
        Init();
    }

    void MaterialPanel::Init()
    {
        auto checkerboard = Renderer::GetCheckerboardTexture();
        s_Instance->m_AlbedoTexture = s_Instance->m_AlbedoTexture ? s_Instance->m_AlbedoTexture : checkerboard;
        s_Instance->m_MetalnessTexture = checkerboard;
        s_Instance->m_RoughnessTexture = checkerboard;
        s_Instance->m_NormalTexture = checkerboard;
    }

    void MaterialPanel::SetMaterial(Ref<Material> material)
    {
        s_Instance->m_Material = material;
        s_Instance->m_PreviewUpdated = true;

        if (!material)
            return;

        auto renderer = ThumbnailRenderer::GetRenderer<MaterialThumbnailRenderer>();
        renderer->SetMaterial(material);
        renderer->Update();
        renderer->ClearCache();

        auto checkerboard = Renderer::GetCheckerboardTexture();
        auto albedoTex = material->GetTexture("u_AlbedoTex");
        auto metalnessTex = material->GetTexture("u_MetalnessTex");
        auto roughnessTex = material->GetTexture("u_RoughnessTex");
        auto normalTex = material->GetTexture("u_NormalTex");
        s_Instance->m_AlbedoTexture = albedoTex ? albedoTex : checkerboard;
        s_Instance->m_MetalnessTexture = metalnessTex ? metalnessTex : checkerboard;
        s_Instance->m_RoughnessTexture = roughnessTex ? roughnessTex : checkerboard;
        s_Instance->m_NormalTexture = normalTex ? normalTex : checkerboard;
    }

    void MaterialPanel::OnImGuiRender()
    {
        if (!s_Show || !m_Material)
            return;

        ImGui::Begin("Material");

        auto renderer = ThumbnailRenderer::GetRenderer<MaterialThumbnailRenderer>();
        if (m_PreviewUpdated)
        {
            if (!renderer->GetCache())
                renderer->Update();
            m_PreviewUpdated = false;
        }
        Ref<Texture2D> preview = renderer->GetOutput();
        RenderPreviewImage(PreviewType::None, preview);

        if (!m_Material)
            m_Material = renderer->GetMaterial();

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
                    RenderPreviewImage(PreviewType::Albedo);

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
                    RenderPreviewImage(PreviewType::Metalness);
                }
                else if (name == "Roughness")
                {
                    RenderPreviewImage(PreviewType::Roughness);
                }
            }

        }
        DrawColumnValue("Normal", [&]() {
            RenderPreviewImage(PreviewType::Normal);
        });

        ImGui::End();
    }

    void MaterialPanel::RenderPreviewImage(PreviewType type, Ref<Texture2D> texture)
    {
        auto checkerboard = Renderer::GetCheckerboardTexture();

        ImGui::InvisibleButton("##thumbnailButton", ImVec2{80, 80});

        UI::BeginDragAndDrop([checkerboard, texture, type, this](AssetHandle handle) {
            Ref<Asset> asset = Application::GetAssetManager()->GetAsset(handle);
            auto& previewTex = GetPreviewTextureByType(type);
            previewTex = asset.As<Texture2D>();

            UpdatePreviewTextureByType(type);
        });

        if (type == PreviewType::None)
            UI::DrawButtonImageByRatio(texture);
        else
            RenderPreviewImageByType(type);
    }

    void MaterialPanel::OnMaterialChange(Ref<Material> material, std::string name, UniformValue value)
    {
        auto renderer = ThumbnailRenderer::GetRenderer<MaterialThumbnailRenderer>();
        renderer->SetMaterialValue(material, name, value);
        renderer->Update();
    }

    void MaterialPanel::OnMaterialChange(Ref<Material> material, std::string name, Ref<Texture2D> texture)
    {
        auto renderer = ThumbnailRenderer::GetRenderer<MaterialThumbnailRenderer>();
        renderer->SetMaterialValue(material, name, texture);
        renderer->Update();
    }
}
