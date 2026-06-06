set(MODULE_NAME CZEditor)

set(MODULE_SRC
    Editor.cpp
    EditorLayer.cpp
    SyncBridge.cpp
    Renderer/Vulkan/VulkanImGuiRenderer.cpp
    EditorNode/EditorNode.cpp
    EditorNode/EditorNodeTree.cpp
    Panels/AssetsPanel.cpp
    Panels/ConsolePanel.cpp
    Panels/TextureViewerPanel.cpp
    Panels/SceneHierarchyPanel.cpp
    Panels/PropertiesPanel.cpp
)

add_library(${MODULE_NAME} SHARED ${MODULE_SRC})

target_link_libraries(${MODULE_NAME} PRIVATE
    CZCoreLibs
    CZRuntimeLibs
    CZVulkan
    Chozo::ImGui
)

target_precompile_headers(${MODULE_NAME} PRIVATE
    EditorPCH.h
)

target_link_options(${MODULE_NAME} PRIVATE -rdynamic)
