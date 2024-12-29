#include "EnvironmentPanel.h"

#include "PropertyUI.h"
#include "TextureViewerPanel.h"

namespace Chozo {

    EnvironmentPanel* EnvironmentPanel::s_Instance;

    float s_LOD;
    glm::vec3 s_LightDirection;
    RendererID s_LightRadiance;
    float s_LightMultiplier;
    float s_Exposure;
    bool s_RadiancePrefiltering;
    float s_EnvMapRotation;
    bool s_ShowBoundingBoxes;

    EnvironmentPanel::EnvironmentPanel()
    {
        s_Instance = this;
    }

    EnvironmentPanel::EnvironmentPanel(const Ref<Scene>& context)
    {
        SetContext(context);
    }

    void EnvironmentPanel::SetContext(const Ref<Scene>& context)
    {
        s_Instance->m_Context = context;
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
            TextureViewerPanel::Open();
            TextureViewerPanel::SetTexture(env->RadianceMap);
        }

        ImGui::End();
    }
}