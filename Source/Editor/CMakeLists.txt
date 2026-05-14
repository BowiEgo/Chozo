set(MODULE_NAME CZEditor)

set(MODULE_SRC
    Editor.cpp
    EditorLayer.cpp
    Renderer/Vulkan/VulkanImGuiRenderer.cpp
)

add_library(${MODULE_NAME} SHARED ${MODULE_SRC})

target_link_libraries(${MODULE_NAME} PRIVATE
    CZCoreLibs
    CZRuntimeLibs
    CZVulkan
    imgui::imgui
)

target_precompile_headers(${MODULE_NAME} PRIVATE
    EditorPCH.h
)

target_link_options(${MODULE_NAME} PRIVATE -rdynamic)
