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
end

local sdk_path = os.getenv("VULKAN_SDK")
if sdk_path then
    sdk_path = path.translate(sdk_path)
end

add_requires("spdlog", "glm")
add_requires("glfw", {configs = {shared = true}})

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

target("VulkanSDK_Interface")
    set_kind("headeronly")
    if sdk_path then
        add_includedirs(path.join(sdk_path, "Include"), {public = true})
        add_linkdirs(path.join(sdk_path, "Lib"), {public = true})
    end

