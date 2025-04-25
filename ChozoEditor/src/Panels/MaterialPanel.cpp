#include "MaterialPanel.h"

#include "Thumbnail/ThumbnailRenderer.h"
#include "PropertyUI.h"

#include <typeindex>
#include <nlohmann/detail/input/parser.hpp>

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
        s_Instance->m_MetallicRoughnessTexture = checkerboard;
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
        auto baseColorMap = material->GetTexture("u_BaseColorMap");
        auto metallicRoughnessMap = material->GetTexture("u_MetallicRoughnessMap");
        auto metallicMap = material->GetTexture("u_MetallicMap");
        auto roughnessMap = material->GetTexture("u_RoughnessMap");
        auto occlusionMap = material->GetTexture("u_AOMap");
        auto emissiveMap = material->GetTexture("u_EmissiveMap");
        auto normalMap = material->GetTexture("u_NormalMap");
        s_Instance->m_BaseColorTexture = baseColorMap ? baseColorMap : checkerboard;
        s_Instance->m_MetallicRoughnessTexture = metallicRoughnessMap ? metallicRoughnessMap : checkerboard;
        s_Instance->m_MetallicTexture = metallicRoughnessMap ? metallicRoughnessMap
            : (metallicMap ? metallicRoughnessMap : checkerboard);
        s_Instance->m_RoughnessTexture = metallicRoughnessMap ? metallicRoughnessMap
            : (roughnessMap ? metallicRoughnessMap : checkerboard);
        s_Instance->m_AOTexture = occlusionMap ? occlusionMap : checkerboard;
        s_Instance->m_EmissiveTexture = emissiveMap ? emissiveMap : checkerboard;
        s_Instance->m_NormalTexture = normalMap ? normalMap : checkerboard;
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
                auto propName = uniform.name;
                auto uniformName = uniform.fullName();
                auto uniformValue = material->GetUniforms()[uniformName];
                auto previewType = StringToPreviewType(propName);

                std::string prefix = "u_Material.";
                size_t prefixPos = uniformName.find(prefix);

                if (prefixPos == std::string::npos)
                    continue;

                std::string label = "##" + propName;

                const auto uniformType = Uniform::GetType(uniformValue);

                if (uniformType == UniformType::Vec4)
                {
                    auto target = Uniform::As<glm::vec4>(uniformValue);
                    DrawColumnValue<glm::vec4>(propName, target, [&](auto& targetVal) {
                        if (ImGui::ColorEdit4(label.c_str(), glm::value_ptr(targetVal)))
                        {
                            material->Set(uniformName, targetVal);
                            OnMaterialChange(uniformName, targetVal);
                        }
                    });
                }

                if (uniformType == UniformType::Vec3)
                {
                    auto target = Uniform::As<glm::vec3>(uniformValue);
                    DrawColumnValue<glm::vec3>(propName, target, [&](auto& targetVal) {
                        if (ImGui::ColorEdit3(label.c_str(), glm::value_ptr(targetVal)))
                        {
                            material->Set(uniformName, targetVal);
                            OnMaterialChange(uniformName, targetVal);
                        }
                    });
                }

                if (uniformType == UniformType::Float)
                {
                    auto target = Uniform::As<float>(uniformValue);
                    DrawColumnValue<float>(propName, target, [&](auto& targetVal) {
                        if (ImGui::DragFloat(label.c_str(), &targetVal, 0.0025f, 0.0f, 1.0f))
                        {
                            material->Set(uniformName, targetVal);
                            OnMaterialChange(uniformName, targetVal);
                        }
                    });
                }

                if (propName == "BaseColor")
                    RenderMap(PreviewType::BaseColor);

                if (propName == "Metallic")
                    RenderMap(PreviewType::Metallic);

                if (propName == "Roughness")
                    RenderMap(PreviewType::Roughness);

                if (propName == "AOIntensity")
                    RenderMap(PreviewType::AO);

                if (propName == "Emissive")
                    RenderMap(PreviewType::Emissive);

                if (propName == "EnableNormalMap") {
                    DrawColumnValue("Normal", [&]() {
                    });
                    RenderMap(PreviewType::Normal);
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
            const Ref<Asset> asset = Application::GetAssetManager()->GetAsset(handle);
            auto& previewTex = GetPreviewTextureByType(type);
            previewTex = asset.As<Texture2D>();

            UpdatePreviewTextureByType(type);
        });

        if (type == PreviewType::None)
            UI::DrawButtonImageByRatio(texture);
        else
            RenderPreviewImageByType(type);
    }

    Ref<Texture2D>& MaterialPanel::GetPreviewTextureByType(const PreviewType type)
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
                bool enableMap = false; \
                if (m_##ENUM##Texture && m_##ENUM##Texture != checkerboard) \
                { \
                    Ref<Texture> texture = m_##ENUM##Texture; \
                    enableMap = true; \
                    material->Set("u_" #ENUM "Map", texture); \
                    material->Set("Enable" #ENUM "Map", enableMap); \
                    OnMaterialChange("u_" #ENUM "Map", texture); \
                    OnMaterialChange("Enable" #ENUM "Map", enableMap); \
                } \
                else \
                { \
                    enableMap = false; \
                    m_##ENUM##Texture = checkerboard; \
                    material->Set("Enable" #ENUM "Map", enableMap); \
                    OnMaterialChange("Enable" #ENUM "Map", enableMap); \
                } \
                break; \
            };
            FOREACH_PREVIEW_TYPE(GENERATE_CASE)
            #undef GENERATE_CASE
            default: return;
        }
    }

    void MaterialPanel::RenderMap(PreviewType type)
    {
        if (type == PreviewType::None)
            return;

        auto material = Application::GetAssetManager()->GetAsset(m_Material).As<Material>();
        if (!material)
            return;

        std::string typeString = PreviewTypeToString(type);
        std::string uniformName = "u_Material.Enable" + typeString + "Map";
        bool enabled = std::get<bool>(material->GetUniforms()[uniformName]);
        // std::string uniformName = "Enable" + typeString + "Map";
        // bool enabled = std::get<bool>(material->GetParamUniforms()[uniformName]);
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