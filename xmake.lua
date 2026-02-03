set_project("ChozoEngine")
set_version("1.0.0")
set_languages("c++20")

add_runenvs("CHOZO_ROOT", os.projectdir())

if is_plat("windows") then
    add_defines("CHOZO_PLATFORM_WINDOWS")
elseif is_plat("linux") then
    add_defines("CHOZO_PLATFORM_LINUX")
elseif is_plat("macosx") then
    add_defines("CHOZO_PLATFORM_MACOS")
end

-- Load the custom module rule defined above
includes("scripts/chozo_module.lua")

-- XMake will look for xmake.lua in each subdirectory
includes("Source/Platform")
includes("Source/Runtime/Core")
includes("Source/Runtime/Engine")
includes("Source/Runtime/Launch")
includes("Source/Runtime/RenderCore")

add_requires("spdlog")
add_requires("glfw")
add_requires("glm")
add_requires("shaderc")
