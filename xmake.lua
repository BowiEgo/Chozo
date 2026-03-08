set_project("ChozoEngine")
set_version("1.0.0")
set_languages("c++20")

add_rules("mode.debug", "mode.release")
if is_mode("debug") then
    set_symbols("debug")
    set_optimize("none")
    add_defines("CZ_DEBUG")
end

if is_mode("release") then
    set_symbols("hidden")  -- Strip symbols for smaller binary size
    set_optimize("fastest") -- Maximize performance
    add_defines("CZ_DIST")
end

add_runenvs("CZ_ROOT", path.absolute(os.projectdir()))

if is_plat("windows") then
    add_defines("CZ_PLATFORM_WINDOWS")
elseif is_plat("linux") then
    add_defines("CZ_PLATFORM_LINUX")
elseif is_plat("macosx") then
    add_defines("CZ_PLATFORM_MACOS")
    add_cxflags("-Wno-parentheses-equality")
end

local sdk_path = os.getenv("VULKAN_SDK")
if sdk_path then
    sdk_path = path.translate(sdk_path)
end

add_requires("spdlog", "glm")
add_requires("glfw", {configs = {shared = true}})
add_requireconfs("*.glfw", {override = true, configs = {shared = true}})

-- Load the custom module rule defined above
includes("scripts/chozo_module.lua")
includes("scripts/export_header.lua")

-- XMake will look for xmake.lua in each subdirectory
includes("External")
includes("Source/Platform")
includes("Source/Runtime/Core")
includes("Source/Runtime/Windowing")
includes("Source/Runtime/VulkanRHI")
includes("Source/Runtime/RHI")
includes("Source/Runtime/VulkanImGui")
includes("Source/Runtime/UI")
includes("Source/Runtime/RenderCore")
includes("Source/Runtime/Engine")
includes("Source/Runtime/Launch")
includes("Source/Editor")
includes("Source/Sandbox")

target("VulkanSDK_Interface")
    set_kind("headeronly")
    if sdk_path then
        -- add_includedirs(path.join(sdk_path, "Include"), {public = true})
        -- add_linkdirs(path.join(sdk_path, "Lib"), {public = true})

        if is_plat("windows") then
            add_includedirs(path.join(sdk_path, "Include"), {public = true})
            add_linkdirs(path.join(sdk_path, "Lib"), {public = true})
            -- add_links("vulkan-1", {public = true})
        else
            add_includedirs(path.join(sdk_path, "include"), {public = true})
            add_rpathdirs(path.join(sdk_path, "lib"), {public = true})
            add_linkdirs(path.join(sdk_path, "lib"), {public = true})
            -- add_links("vulkan", {public = true}) -- [Note] On Mac, link to libvulkan.dylib
        end
    end

target("CopyBinaries")
    set_kind("phony") -- Does not compile any code

    -- Ensure this runs after the core modules are built
    add_deps("RenderCore", "Windowing")

    after_build(function (target)
        import("core.base.option")
        
        -- The directory where your final executable stays (e.g., bin/windows/x64/debug)
        local outdir = target:targetdir()
        -- local dlls = {}
        local bin_files = {}
        local is_win = is_plat("windows")

        local vulkan_sdk_path = os.getenv("VULKAN_SDK")
        if vulkan_sdk_path then
            local shaderc_name = is_win and "shaderc_shared.dll" or "libshaderc_shared.dylib"
            local shaderc_path = path.join(sdk_path, is_win and "Bin" or "lib", shaderc_name)
            table.insert(bin_files, shaderc_path)
        else
            cprint("${yellow}[CopyBinaries]:${clear} Warning - VULKAN_SDK environment variable not found!")
        end

        local windowing = target:dep("Windowing")
        if windowing then
            local glfw_pkg = windowing:pkg("glfw")
            if glfw_pkg then
                local pkg_dir = glfw_pkg:installdir()
                local lib_name = is_win and "glfw3.dll" or "libglfw.3.dylib"
                local search_paths = {path.join(pkg_dir, "bin"), path.join(pkg_dir, "lib")}
                local found = false
                for _, p in ipairs(search_paths) do
                    local full_path = path.join(p, lib_name)
                    if os.isfile(full_path) then
                        table.insert(bin_files, full_path)
                        found = true
                        break
                    end
                end

                if not found then
                    cprint("${yellow}[CopyBinaries]:${clear} Warning - glfw3.dll not found in pkg dir: %s", pkg_dir)
                end
            end
        end

        for _, src in ipairs(bin_files) do
            local filename = path.filename(src)
            local dst = path.join(outdir, filename)
            
            if os.isfile(src) then
                if not os.isfile(dst) or os.mtime(src) > os.mtime(dst) then
                    cprint("${green}[CopyBinaries]:${clear} updating %s", filename)
                    os.cp(src, dst)
                end
            else
                cprint("${yellow}[CopyBinaries]:${clear} Warning - source file missing: %s", src)
            end
        end
    end)