#include "AssetsPanel.h"

#include "Chozo/Core/Application.h"

namespace Chozo {

    AssetsPanel* AssetsPanel::s_Instance;

    AssetsPanel::AssetsPanel()
    {
        s_Instance = this;
    }

    void AssetsPanel::OnImGuiRender()
    {
        ImGui::Begin("Assets");

        const auto textureHandles = Application::GetAssetManager()->GetAllAssetsWithType(AssetType::Texture);
        const auto meshHandles = Application::GetAssetManager()->GetAllAssetsWithType(AssetType::MeshSource);
        const auto materialHandles = Application::GetAssetManager()->GetAllAssetsWithType(AssetType::Material);

        ImGui::Text("Textures");
        for (const auto& handle : textureHandles)
        {
            ImGui::Text("%s", std::to_string(handle).c_str());
        }

        ImGui::Spacing();
        ImGui::Text("Meshes");

        for (const auto& handle : meshHandles)
        {
            ImGui::Text("%s", std::to_string(handle).c_str());
        }

        ImGui::Spacing();
        ImGui::Text("Materials");

        for (const auto& handle : materialHandles)
        {
            ImGui::Text("%s", std::to_string(handle).c_str());
        }

        ImGui::End();
    }
}