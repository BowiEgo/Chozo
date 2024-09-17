#include "EnvironmentPanel.h"

#include "PropertyUI.h"
#include "TexturePreviewPanel.h"

#include <regex>

namespace Chozo {

    extern const std::filesystem::path g_AssetsPath;
    extern const std::regex imagePattern;

    float s_LOD;
    glm::vec3 s_LightDirection;
    RendererID s_LightRadiance;
    float s_LightMultiplier;
    float s_Exposure;
    bool s_RadiancePrefiltering;
    float s_EnvMapRotation;
    bool s_ShowBoundingBoxes;
    
    EnvironmentPanel::EnvironmentPanel(const Ref<Scene> context)
    {
        SetContext(context);
    }

    void EnvironmentPanel::SetContext(const Ref<Scene> context)
    {
        m_Context = context;
    }

    void EnvironmentPanel::OnImGuiRender()
    {
        ImGui::Begin("Environment");

        if (!m_Context)
        {
            ImGui::End();
            return;
        }

        Ref<Environment> env = m_Context->GetEnvironment();

        ImGui::Button("Path");
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                const wchar_t* path = (const wchar_t*)payload->Data;
                CZ_INFO("Drop target: {0}", (char*)path);
                std::filesystem::path filePath = std::filesystem::path((char*)path);
                std::string fileExtension = filePath.extension().string();
                if (std::regex_match(fileExtension, imagePattern))
                {
                    std::filesystem::path texturePath = g_AssetsPath / std::filesystem::path((char*)path);

                    TextureSpecification spec;
                    spec.WrapS = ImageParameter::CLAMP_TO_BORDER;
                    spec.WrapT = ImageParameter::CLAMP_TO_BORDER;
                    Ref<Texture2D> envMap = Texture2D::Create(texturePath.string(), spec);
                    TexturePreviewPanel::Open();
                    TexturePreviewPanel::SetTexture(envMap);
                }
            }

            ImGui::EndDragDropTarget();
        }

        DrawColumnValue<float>("Skybox LOD", s_LOD, [&](auto& target) {
            if (ImGui::DragFloat("##Skybox LOD", &target, 0.1f, 0.0f, 10.0f))
                s_LOD = target;
        });

        DrawColumnValue<glm::vec3>("Light Direction", s_LightDirection, [&](auto& target) {
            DrawVec3Control("Light Direction", s_LightDirection, 0.0f, 1.0f);
        });

        DrawColumnValue<float>("Light Multiplier", s_LightMultiplier, [&](auto& target) {
            if (ImGui::SliderFloat("##Light Multiplier", &target, 0.0f, 10.0f))
                s_LightMultiplier = target;
        });

        DrawColumnValue<float>("Exposure", s_Exposure, [&](auto& target) {
            if (ImGui::SliderFloat("##Exposure", &target, 0.0f, 10.0f))
                s_Exposure = target;
        });

        DrawColumnValue<bool>("Radiance Prefiltering", s_RadiancePrefiltering, [&](auto& target) {
            ImGui::Checkbox("##Radiance Prefiltering", &target);
        });

        DrawColumnValue<float>("EnvMapRotation", s_EnvMapRotation, [&](auto& target) {
            if (ImGui::DragFloat("##EnvMapRotation", &target, 0.1f, 0.0f, 10.0f))
                s_EnvMapRotation = target;
        });

        DrawColumnValue<bool>("Show Bounding Boxes", s_ShowBoundingBoxes, [&](auto& target) {
            ImGui::Checkbox("##Show Bounding Boxes", &target);
        });

        if(ImGui::Button("ShowRadianceMap") && env)
        {
            TexturePreviewPanel::Open();
            TexturePreviewPanel::SetTexture(env->RadianceMap);
        }

        ImGui::End();
    }
}