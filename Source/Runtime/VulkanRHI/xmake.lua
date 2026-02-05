target("VulkanRHI")
    set_kind("shared")
    add_rules("chozo_module", "export_header")
    add_deps("Core", "RHI")

    add_packages("shaderc")

    on_load(function (target)
        local sdk_path = os.getenv("VULKAN_SDK")

        if not sdk_path then
            raise("Vulkan SDK not found! Please check VULKAN_SDK environment variable.")
        end
    
        sdk_path = path.translate(sdk_path)

        target:add("includedirs", path.join(sdk_path, "Include"))
    end)

    if is_plat("windows") then
        add_defines("VK_USE_PLATFORM_WIN32_KHR")
    elseif is_plat("linux") then
        add_defines("VK_USE_PLATFORM_XCB_KHR")
    elseif is_plat("macosx") then
        add_defines("VK_USE_PLATFORM_METAL_EXT")
    end
