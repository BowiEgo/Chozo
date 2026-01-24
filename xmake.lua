set_project("ChozoEngine")
set_version("1.0.0")
set_languages("c++20")

-- Load the custom module rule defined above
includes("scripts/chozo_module.lua")

-- XMake will look for xmake.lua in each subdirectory
includes("Source/Runtime/**")

add_requires("spdlog")
add_requires("glfw")