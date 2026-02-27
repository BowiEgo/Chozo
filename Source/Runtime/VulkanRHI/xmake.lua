target("VulkanRHI")
    set_kind("shared")
    add_rules("chozo_module", "export_header")
    add_deps("Core", "Platform", "RHI", "VulkanSDK_Interface")

    if is_plat("windows") then
        add_links("vulkan-1", {public = true})
        add_defines("VK_USE_PLATFORM_WIN32_KHR")
    elseif is_plat("linux") then
        add_defines("VK_USE_PLATFORM_XCB_KHR")
    elseif is_plat("macosx") then
        add_links("vulkan", {public = true})
        add_defines("VK_USE_PLATFORM_METAL_EXT")
    end
    