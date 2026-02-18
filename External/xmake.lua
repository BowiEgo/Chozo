target("imgui")
    set_kind("shared")

    add_defines("IMGUI_DEFINE_MATH_OPERATORS", {public = true})
    
    add_files("imgui/*.cpp")
    add_files("imgui/backends/imgui_impl_glfw.cpp")
    add_files("imgui/backends/imgui_impl_vulkan.cpp")
    add_files("imgui/misc/cpp/imgui_stdlib.cpp")

    -- Export include paths so other modules can #include "imgui.h"
    add_includedirs("imgui", "imgui/backends", "imgui/misc/cpp", {public = true})

    -- Crucial! Link against your system Vulkan and GLFW
    -- This resolves the LNK2019 errors once and for all.
    add_deps("VulkanSDK_Interface", {public = true})
    add_links("vulkan-1", {public = true})

    add_packages("glfw", {public = true})

    -- Windows DLL symbol export handling
    if is_plat("windows") then
        add_defines("IMGUI_API=__declspec(dllexport)", {public = true})
    end

    -- Speed up build using Precompiled Headers
    set_pcxxheader("imgui/imgui.h")

    -- Prevent re-scanning and speed up linking if imgui doesn't change
    set_policy("package.librarydeps.strict_compatibility", false)