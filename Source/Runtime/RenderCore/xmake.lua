target("RenderCore")
    set_kind("shared")
    add_rules("chozo_module", "export_header")
    add_deps("Core", "FileSystem", "RHI", "VulkanSDK_Interface", "spirv-reflect")
    add_packages("spdlog", "glm", "entt", { public = true })

    if is_plat("windows") then
        -- Use shaderc_combined (static) or shaderc_shared (DLL)
        -- 'shaderc_combined' is usually preferred to avoid carrying extra DLLs.
        add_links("shaderc_shared", "spirv-cross-core", "spirv-cross-glsl", "spirv-reflect")
    else
        add_links("shaderc_shared", "spirv-cross-core", "spirv-cross-glsl", "spirv-reflect")
    end
