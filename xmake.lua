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
    add_defines("NOMINMAX")
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

add_requires("glm", "entt")
add_requires("spdlog", {configs = {header_only = false}})
add_requires("libsdl3", {configs = {shared = true}})

option("tests")
    set_default(false)
    set_showmenu(true)
    set_description("Build and run tests")

function is_test_mode()
    return get_config("tests") or false
end

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
includes("Source/Runtime/FileSystem")
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
            add_links("vulkan-1", {public = true})
        else
            add_includedirs(path.join(sdk_path, "include"), {public = true})
            add_rpathdirs(path.join(sdk_path, "lib"), {public = true})
            add_linkdirs(path.join(sdk_path, "lib"), {public = true})
            add_links("vulkan", {public = true})
        end
    end

target("CopyFiles")
    set_kind("phony") -- Does not compile any code

    -- Ensure this runs after the core modules are built
    add_deps("RenderCore", "Windowing")

    after_build(function (target)
        import("core.project.project")

        -- The directory where your final executable stays (e.g., bin/windows/x64/release)
        local outdir = target:targetdir()
        local bin_files = {}
        local is_win = is_plat("windows")

        local config_src = path.join(os.projectdir(), "Config", "imgui.ini")
        local config_dst = path.join(outdir, "imgui.ini")

        if os.isfile(config_src) then
            if not os.isfile(config_dst) or os.mtime(config_src) > os.mtime(config_dst) then
                cprint("${green}[CopyFiles]:${clear} updating imgui.ini")
                os.cp(config_src, config_dst)
            end
        else
            cprint("${yellow}[CopyFiles]:${clear} Warning - imgui.ini not found at %s", config_src)
        end

        local folders = {"Resources", "Shaders", "Config"}

        for _, folder in ipairs(folders) do
            local src = path.join(os.projectdir(), folder)
            local dst = path.join(outdir, folder)
            
            if os.isdir(src) then
                os.cp(src, outdir) 
                cprint("${green}[CopyBinaries]:${clear} deploying folder %s", folder)
            end
        end

        for _, depname in ipairs(target:get("deps")) do
            local dep = project.target(depname)
            if dep then
                for _, pkg in pairs(dep:pkgs()) do
                    local libfiles = pkg:get("libfiles")
                    if libfiles then
                        for _, libfile in ipairs(libfiles) do
                            if (is_win and libfile:find("%.dll$")) or 
                               (not is_win and libfile:find("%.dylib$")) then
                                table.insert(bin_files, libfile)
                            end
                        end
                    end
                end
            end
        end

        local vulkan_sdk_path = os.getenv("VULKAN_SDK")
        if vulkan_sdk_path then
            local shaderc_name = is_win and "shaderc_shared.dll" or "libshaderc_shared.dylib"
            local shaderc_path = path.join(sdk_path, is_win and "Bin" or "lib", shaderc_name)
            table.insert(bin_files, shaderc_path)
        else
            cprint("${yellow}[CopyFiles]:${clear} Warning - VULKAN_SDK environment variable not found!")
        end

        for _, src in ipairs(bin_files) do
            local filename = path.filename(src)
            local dst = path.join(outdir, filename)
            
            if os.isfile(src) then
                if not os.isfile(dst) or os.mtime(src) > os.mtime(dst) then
                    cprint("${green}[CopyFiles]:${clear} updating %s", filename)
                    os.cp(src, dst)
                end
            else
                cprint("${yellow}[CopyFiles]:${clear} Warning - source file missing: %s", src)
            end
        end
    end)

if is_test_mode() then
    includes("Source/Test")
end