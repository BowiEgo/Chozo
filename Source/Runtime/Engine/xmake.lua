target("Engine")
set_kind("shared")
add_rules("chozo_module")
add_deps("Core")
add_deps("RenderCore")
add_packages("glfw")
add_defines("VULKAN_HPP_NO_SPACESHIP_OPERATOR")

on_load(function(target)
    local sdk_path = os.getenv("VULKAN_SDK")

    -- if VULKAN_SDK is not set, try to find it in common locations
    if not sdk_path then
        -- common Windows default installation paths
        local common_paths = os.dirs("C:/VulkanSDK/*")
        if common_paths and #common_paths > 0 then
            sdk_path = common_paths[#common_paths] -- get the latest version
        end
    end

    if sdk_path then
        target:add("includedirs", sdk_path .. "/Include")
        target:add("linkdirs", sdk_path .. "/Lib")
        target:add("links", "vulkan-1")
        print("[Chozo] Found Vulkan SDK at: " .. sdk_path)
    else
        raise("Vulkan SDK not found! Run setup.bat or set VULKAN_SDK env.")
    end
end)
