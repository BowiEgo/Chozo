#include "MaterialPanel.h"

#include "Thumbnail/ThumbnailRenderer.h"
#include "PropertyUI.h"

#include <typeindex>

namespace Chozo {

	MaterialPanel* MaterialPanel::s_Instance;
    bool MaterialPanel::s_Show = false;

    std::vector<std::string> MaterialPropTypes = {
        "BaseColor",
        "Metallic",
        "Roughness",
        "Normal",
        "Ambient",
        "AmbientStrength",
        "Specular",
        "EnableBaseColorTex",
        "EnableMetallicTex",
        "EnableRoughnessTex",
        "EnableNormalTex",
    };

    MaterialPanel::MaterialPanel()
    {
        s_Instance = this;
        Init();
    }

    void MaterialPanel::Init()
    {
        auto checkerboard = Renderer::GetCheckerboardTexture();
        s_Instance->m_BaseColorTexture = s_Instance->m_BaseColorTexture ? s_Instance->m_BaseColorTexture : checkerboard;
        s_Instance->m_MetallicTexture = checkerboard;
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
        auto baseColorTex = material->GetTexture("u_BaseColorTex");
        auto metallicTex = material->GetTexture("u_MetallicTex");
        auto roughnessTex = material->GetTexture("u_RoughnessTex");
        auto normalTex = material->GetTexture("u_NormalTex");
        s_Instance->m_BaseColorTexture = baseColorTex ? baseColorTex : checkerboard;
        s_Instance->m_MetallicTexture = metallicTex ? metallicTex : checkerboard;
        s_Instance->m_RoughnessTexture = roughnessTex ? roughnessTex : checkerboard;
        s_Instance->m_NormalTexture = normalTex ? normalTex : checkerboard;
    }

    void MaterialPanel::OnImGuiRender()
    {
        if (!s_Show || !m_Material)
            return;

        auto renderer = ThumbnailRenderer::GetRenderer<MaterialThumbnailRenderer>();
        // ImVec2 windowPadding = ImGui::GetStyle().WindowPadding;
		UI::ScopedStyle padding(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Material");

        // Preview
        {
            if (m_PreviewUpdated)
            {
                if (!renderer->GetCache())
                    renderer->Update();
                m_PreviewUpdated = false;
            }

            Ref<Texture2D> preview = renderer->GetOutput();
            RenderPreviewImage(PreviewType::None, preview);
        }

        // Properties
        {
            // UI::ScopedStyle padding(ImGuiStyleVar_WindowPadding, windowPadding);
            ImGui::BeginChild("ScrollableArea", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar);

            if (!m_Material)
                m_Material = renderer->GetMaterial();

            for (auto propName : MaterialPropTypes)
            {
                std::string uniformName = "u_Material." + propName;
                auto value = m_Material->GetUniforms()[uniformName];
                auto previewType = StringToPreviewType(propName);

                if (std::holds_alternative<glm::vec3>(value))
                {
                    auto& target = std::get<glm::vec3>(value);
                    DrawColumnValue<glm::vec3>(propName, target, [&](auto& targetVal) {
                        if (ImGui::ColorEdit3(("##" + propName).c_str(), glm::value_ptr(targetVal)))
                        {
                            m_Material->Set(uniformName, targetVal);
                            OnMaterialChange(m_Material, uniformName, targetVal);
                        }
                    });

                    RenderTextureProp(previewType);
                }

                if (propName == "Normal")
                {
                    DrawColumnValue("Normal", [&]() {
                    });
                    RenderTextureProp(PreviewType::Normal);
                }

                if (std::holds_alternative<float>(value))
                {
                    auto& target = std::get<float>(value);
                    DrawColumnValue<float>(propName, target, [&](auto& targetVal) {
                        if (ImGui::DragFloat(("##" + propName).c_str(), &targetVal, 0.0025f, 0.0f, 1.0f))
                        {
                            m_Material->Set(uniformName, targetVal);
                            OnMaterialChange(m_Material, uniformName, targetVal);
                        }
                    });

                    RenderTextureProp(previewType);
                }
            }
        }

        ImGui::EndChild();
        ImGui::End();
    }

    void MaterialPanel::RenderPreviewImage(PreviewType type, const Ref<Texture2D>& texture)
    {
        auto checkerboard = Renderer::GetCheckerboardTexture();

        ImGui::InvisibleButton("##thumbnailButton", ImVec2{80, 80});

        UI::BeginDragAndDrop([type, this](const AssetHandle handle) {
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

    Ref<Texture2D>& MaterialPanel::GetPreviewTextureByType(PreviewType type)
    {
        switch (type) {
            #define GENERATE_CASE(ENUM) case PreviewType::ENUM: return m_##ENUM##Texture;
            FOREACH_PREVIEW_TYPE(GENERATE_CASE)
            #undef GENERATE_CASE
            default: return m_BaseColorTexture;
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
                    m_Material->Set("u_Material.Enable" #ENUM "Tex", true); \
                    OnMaterialChange(m_Material, "u_" #ENUM "Tex", m_##ENUM##Texture); \
                    OnMaterialChange(m_Material, "u_Material.Enable" #ENUM "Tex", true); \
                } \
                else \
                { \
                    m_##ENUM##Texture = checkerboard; \
                    m_Material->Set("u_Material.Enable" #ENUM "Tex", false); \
                    OnMaterialChange(m_Material, "u_Material.Enable" #ENUM "Tex", false); \
                } \
                break; \
            };
            FOREACH_PREVIEW_TYPE(GENERATE_CASE)
            #undef GENERATE_CASE
            default: return;
        }
    }

    void MaterialPanel::RenderTextureProp(PreviewType type)
    {
        if (type == PreviewType::None)
            return;

        std::string typeString = PreviewTypeToString(type);
        std::string uniformName = "u_Material.Enable" + typeString + "Tex";
        bool enabled = std::get<bool>(m_Material->GetUniforms()[uniformName]);
        bool changed = false;

        DrawColumnImage("##", enabled, changed, [type, this]() {
            RenderPreviewImage(type);
        });

        if (changed)
        {
            m_Material->Set(uniformName, enabled);
            OnMaterialChange(m_Material, uniformName, enabled);
        }
    }

    void MaterialPanel::RenderPreviewImageByType(PreviewType type) const
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

    void MaterialPanel::OnMaterialChange(const Ref<Material>& material, std::string name, UniformValue value)
    {
        auto renderer = ThumbnailRenderer::GetRenderer<MaterialThumbnailRenderer>();
        renderer->SetMaterialValue(material, std::move(name), std::move(value));
        renderer->Update();
    }

    void MaterialPanel::OnMaterialChange(const Ref<Material>& material, std::string name, const Ref<Texture2D>& texture)
    {
        auto renderer = ThumbnailRenderer::GetRenderer<MaterialThumbnailRenderer>();
        renderer->SetMaterialValue(material, std::move(name), texture);
        renderer->Update();
    }
}
