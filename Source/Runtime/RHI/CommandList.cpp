#include <Runtime/RHI/CommandList.hpp>
#include <Runtime/RHI/RHIAPI.hpp>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogCommandList, Info);

DEFINE_HANDLE_DESTROY(CommandListObj)

void CommandListObj::Draw(Scene scene, Camera camera) {
    {
        auto setLayout = m_CurrentPipeline->GetSetLayout(0);

        if (setLayout) {
            auto cameraBuffer = CameraManager::Get().GetCameraBuffer(camera.Raw());

            std::vector<DescriptorBinding> bindings = {
                { 0, ResourceType::GraphicsBuffer, cameraBuffer },
            };
            auto descSet =
                RHIAPI::Get()->GetGraphicsContext()->GetDevice()->GetOrCreateDescriptorSet(
                    setLayout, bindings);

            BindDescriptorSets(0, descSet);
        }
    }

    auto renderDatas = scene->GetRenderDatas();
    for (auto& [pushConstants, mesh] : renderDatas) {
        if (mesh) {
            // if (material) {
            //     BindMaterial(material);
            // }
            PushConstants(&pushConstants, sizeof(pushConstants), 0);
            Draw(mesh);
        }
    }
}

} // namespace CZ