target("VulkanRHI")
    set_kind("shared")
    add_rules("chozo_module")
    add_deps("Core", "RHI")

    add_packages("shaderc")

    on_load(function (target)
        local sdk_path = os.getenv("VULKAN_SDK")

        if not sdk_path then
            raise("Vulkan SDK not found! Please check VULKAN_SDK environment variable.")
        end
    
        sdk_path = path.translate(sdk_path)

        target:add("includedirs", path.join(sdk_path, "Include"))
        -- Use an absolute path to link vulkan-1.lib directly.
        -- This prevents the linker from searching the entire SDK/Lib directory,
        -- effectively avoiding conflicts with the SDK's built-in shaderc libraries.
        target:add("links", path.join(sdk_path, "Lib", "vulkan-1.lib"))
    end)

    if is_plat("windows") then
        add_defines("VK_USE_PLATFORM_WIN32_KHR")
    elseif is_plat("linux") then
        add_defines("VK_USE_PLATFORM_XCB_KHR")
    elseif is_plat("macosx") then
        add_defines("VK_USE_PLATFORM_METAL_EXT")
    end
