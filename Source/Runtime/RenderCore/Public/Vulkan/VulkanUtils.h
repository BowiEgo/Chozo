#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Chozo::VulkanUtils {

    namespace {
#ifdef NDEBUG
        constexpr bool EnableValidationLayers = false;
#else
        constexpr bool EnableValidationLayers = true;
#endif
    }

    inline const std::vector<const char*> ValidationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    
    template <typename T, typename Getter>
    inline bool IsSupported(const std::vector<const char*>& required, const std::vector<T>& available, Getter&& nameGetter) {
        return std::ranges::all_of(required, [&](const char* name) {
            return std::ranges::any_of(available, [&](const T& item) {
                return std::string_view(nameGetter(item)) == name;
            });
        });
    }

    inline bool CheckValidationLayerSupport(vk::raii::Context& context) {
        auto available = context.enumerateInstanceLayerProperties();
        return IsSupported(ValidationLayers, available, [](auto& l){ return l.layerName; });
    }

    inline bool CheckInstanceExtensions(const vk::raii::Context& context, const std::vector<const char*>& required) {
        auto available = context.enumerateInstanceExtensionProperties();
        return IsSupported(required, available, [](const auto& p) { return p.extensionName; });
    }
}