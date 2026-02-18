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

    -- English Comment: If you chose 'shaderc_shared' (DLL), copy it to the target directory.
    -- If you use 'shaderc_combined', you can remove this part.
    after_build(function (target)
        local sdk_path = os.getenv("VULKAN_SDK")
        if sdk_path then
            local bin_dir = path.join(sdk_path, "Bin")
            local dll_path = path.join(bin_dir, "shaderc_shared.dll")
            if os.exists(dll_path) then
                os.cp(dll_path, target:targetdir())
            end
        end
    end)

    -- on_load(function (target)
    --     local sdk_path = os.getenv("VULKAN_SDK")
    --     if not sdk_path then
    --         raise("Vulkan SDK not found!")
    --     end
    --     sdk_path = path.translate(sdk_path)

    --     -- [Note] Add Include paths specifically for shaderc and spirv-cross
    --     -- Most SDKs put these under the main Include folder
    --     target:add("includedirs", path.join(sdk_path, "Include"))

    --     -- [Note] Link ONLY the specific shader/spirv libraries
    --     -- On Windows, these are typically:
    --     if is_plat("windows") then
    --         local lib_dir = path.join(sdk_path, "Lib")
    --         target:add("links", path.join(lib_dir, "shaderc_shared.lib"))
    --         target:add("links", path.join(lib_dir, "spirv-cross-core.lib"))
    --         target:add("links", path.join(lib_dir, "spirv-cross-glsl.lib"))
    --     else
    --         -- Linux/macOS paths (.a / .so)
    --         target:add("links", "shaderc_shared", "spirv-cross-core", "spirv-cross-glsl")
    --     end
    -- end)

    -- after_build(function (target)
    --     local sdk_path = os.getenv("VULKAN_SDK")
    --     local bin_dir = path.join(sdk_path, "Bin")
    --     -- [Note] Copy the required DLL to the output directory
    --     os.cp(path.join(bin_dir, "shaderc_shared.dll"), target:targetdir())
    -- end)