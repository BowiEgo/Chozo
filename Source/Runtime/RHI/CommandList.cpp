#include <Core/Log/LogMacros.hpp>
#include <Runtime/RHI/CommandList.hpp>
#include <Runtime/RHI/RHIAPI.hpp>

namespace CZ {

DEFINE_LOG_CATEGORY_STATIC(LogCommandList, Info);

namespace {

/// Reports a condition once per process so that a broken draw setup is visible in the log
/// without flooding it at frame rate.
bool WarnOnce(bool& alreadyReported, const char* format) {
    if (alreadyReported) return false;

    alreadyReported = true;
    CZ_RHI_LOG(Warning, "{}", format);
    return true;
}

} // namespace

void CommandListObj::Draw(Scene scene, Camera camera) {
    if (!scene) {
        static bool warned = false;
        WarnOnce(warned, "CommandList::Draw(Scene, Camera) called with a null scene; skipping.");
        return;
    }

    if (!m_CurrentPipeline) {
        static bool warned = false;
        WarnOnce(warned, "CommandList::Draw(Scene, Camera) called without a bound pipeline; "
                         "skipping.");
        return;
    }

    // Set 0 carries the per-view data (the camera buffer). Binding nothing there leaves the
    // shader reading an undefined descriptor, which the validation layers report as a descriptor
    // error far away from the actual cause, so say it here instead.
    SetLayout setLayout = m_CurrentPipeline->GetSetLayout(0);

    if (!setLayout) {
        static bool warned = false;
        WarnOnce(warned, "The bound pipeline exposes no set 0; per-view data (camera) will not be "
                         "bound and draws are likely to fail validation.");
    } else {
        GraphicsBuffer cameraBuffer = CameraManager::Get().GetCameraBuffer(camera.Raw());

        if (!cameraBuffer) {
            static bool warned = false;
            WarnOnce(warned, "The camera is not registered with the CameraManager; set 0 will be "
                             "bound without a camera buffer.");
        }

        std::vector<DescriptorBinding> bindings = {
            { 0, ResourceType::GraphicsBuffer, cameraBuffer },
        };

        DescriptorSet descSet =
            RHIAPI::Get()->GetGraphicsContext()->GetDevice()->GetOrCreateDescriptorSet(setLayout,
                                                                                       bindings);

        BindDescriptorSets(0, descSet);
    }

    for (auto& [pushConstants, mesh] : scene->GetRenderDatas()) {
        if (!mesh) continue;

        PushConstants(&pushConstants, sizeof(pushConstants), 0);
        Draw(mesh);
    }
}

} // namespace CZ
