target("VulkanImGui")
    set_kind("shared")
    add_rules("chozo_module", "export_header")
    add_deps("Core", "UI", "Vulkan")
    