#include "MaterialPanel.h"

#include "Thumbnail/ThumbnailRenderer.h"
#include "PropertyUI.h"

#include <typeindex>

namespace Chozo {

	MaterialPanel* MaterialPanel::s_Instance;
    bool MaterialPanel::s_Show = false;

    MaterialPanel::MaterialPanel()
    {
        s_Instance = this;
        Init();
    }

    void MaterialPanel::Init()
    {
        const auto checkerboard = Renderer::GetCheckerboardTexture();
        s_Instance->m_BaseColorTexture = s_Instance->m_BaseColorTexture ? s_Instance->m_BaseColorTexture : checkerboard;
        s_Instance->m_MetallicTexture = checkerboard;
        s_Instance->m_RoughnessTexture = checkerboard;
        s_Instance->m_NormalTexture = checkerboard;
    }

    void MaterialPanel::SetMaterial(AssetHandle handle)
    {
        s_Instance->m_Material = handle;

        if (!handle)
            return;

        auto material = Application::GetAssetManager()->GetAsset(handle).As<Material>();
        if (!material)
            return;

        const auto renderer = ThumbnailRenderer::GetRenderer<MaterialThumbnailRenderer>();
        renderer->SetMaterial(material);
        renderer->Update();

        const auto checkerboard = Renderer::GetCheckerboardTexture();
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

        auto material = Application::GetAssetManager()->GetAsset(m_Material).As<Material>();
        if (!material)
            return;

        auto renderer = ThumbnailRenderer::GetRenderer<MaterialThumbnailRenderer>();
        // ImVec2 windowPadding = ImGui::GetStyle().WindowPadding;
		UI::ScopedStyle padding(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Material");

        // Preview
        {
            Ref<Texture2D> preview = renderer->GetOutput();
            RenderPreviewImage(PreviewType::None, preview);
        }

        // Properties
        {
            // UI::ScopedStyle padding(ImGuiStyleVar_WindowPadding, windowPadding);
            ImGui::BeginChild("ScrollableArea", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar);

            if (!material)
                material = renderer->GetMaterial();

            for (const auto& uniform : material->GetShader()->GetReflection().uniforms)
            {
                if (uniform.resourceName != "u_Material")
                    continue;

                auto uniformName = uniform.name;
                auto fullName = uniform.fullName();
                auto value = material->GetUniforms()[fullName];
                auto previewType = StringToPreviewType(uniformName);

                if (std::holds_alternative<glm::vec3>(value))
                {
                    auto& target = std::get<glm::vec3>(value);
                    DrawColumnValue<glm::vec3>(uniformName, target, [&](auto& targetVal) {
                        if (ImGui::ColorEdit3(("##" + uniformName).c_str(), glm::value_ptr(targetVal)))
                        {
                            material->Set(fullName, targetVal);
                            OnMaterialChange(fullName, targetVal);
                        }
                    });

                    RenderTextureProp(previewType);
                }

                if (uniformName == "Normal")
                {
                    DrawColumnValue("Normal", [&]() {
                    });
                    RenderTextureProp(PreviewType::Normal);
                }

                if (std::holds_alternative<float>(value))
                {
                    auto& target = std::get<float>(value);
                    DrawColumnValue<float>(uniformName, target, [&](auto& targetVal) {
                        if (ImGui::DragFloat(("##" + uniformName).c_str(), &targetVal, 0.0025f, 0.0f, 1.0f))
                        {
                            material->Set(fullName, targetVal);
                            OnMaterialChange(fullName, targetVal);
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
        auto material = Application::GetAssetManager()->GetAsset(m_Material).As<Material>();
        if (!material)
            return;

        switch (type) {
            #define GENERATE_CASE(ENUM) case PreviewType::ENUM: { \
                auto checkerboard = Renderer::GetCheckerboardTexture(); \
                if (m_##ENUM##Texture && m_##ENUM##Texture != checkerboard) \
                { \
                    material->Set("u_" #ENUM "Tex", m_##ENUM##Texture); \
                    material->Set("u_Material.Enable" #ENUM "Tex", true); \
                    OnMaterialChange("u_" #ENUM "Tex", m_##ENUM##Texture); \
                    OnMaterialChange("u_Material.Enable" #ENUM "Tex", true); \
                } \
                else \
                { \
                    m_##ENUM##Texture = checkerboard; \
                    material->Set("u_Material.Enable" #ENUM "Tex", false); \
                    OnMaterialChange("u_Material.Enable" #ENUM "Tex", false); \
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

        auto material = Application::GetAssetManager()->GetAsset(m_Material).As<Material>();
        if (!material)
            return;

        std::string typeString = PreviewTypeToString(type);
        std::string uniformName = "u_Material.Enable" + typeString + "Tex";
        bool enabled = std::get<bool>(material->GetUniforms()[uniformName]);
        bool changed = false;

        DrawColumnImage("##", enabled, changed, [type, this]() {
            RenderPreviewImage(type);
        });

        if (changed)
        {
            material->Set(uniformName, enabled);
            OnMaterialChange(uniformName, enabled);
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

    void MaterialPanel::OnMaterialChange(const std::string& name, const MaterialProp& value)
    {
        const auto renderer = ThumbnailRenderer::GetRenderer<MaterialThumbnailRenderer>();
        std::visit([&](const auto& v) {
            renderer->SetMaterialProp(name, v);
        }, value);
    }
}
