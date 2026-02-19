target("RenderCore")
    set_kind("shared")
    add_rules("chozo_module", "export_header")
    add_deps("Core", "RHI", "VulkanSDK_Interface")

    if is_plat("windows") then
        -- English Comment: Use shaderc_combined (static) or shaderc_shared (DLL)
        -- 'shaderc_combined' is usually preferred to avoid carrying extra DLLs.
        add_links("shaderc_shared", "spirv-cross-core", "spirv-cross-glsl")
    else
        add_links("shaderc_shared", "spirv-cross-core", "spirv-cross-glsl")
    end
