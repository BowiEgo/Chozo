set(MODULE_NAME CZEditor)

set(MODULE_SRC
    Editor.cpp
    EditorLayer.cpp
    SyncBridge.cpp
    EditorCamera.cpp
    EditorNode/EditorNode.cpp
    EditorNode/EditorNodeTree.cpp
    Panels/ConsolePanel.cpp
    Panels/SceneHierarchyPanel.cpp
    Panels/PropertiesPanel.cpp
)

add_library(${MODULE_NAME} SHARED ${MODULE_SRC})

# The editor talks to the graphics backend exclusively through public interfaces
# (`UIRenderBackendObj`, RHI); the backend module itself is loaded at runtime.
target_link_libraries(${MODULE_NAME} PRIVATE
    CZCoreLibs
    CZRuntimeLibs
    Chozo::ImGui
)

target_precompile_headers(${MODULE_NAME} PRIVATE
    EditorPCH.h
)

target_link_options(${MODULE_NAME} PRIVATE -rdynamic)

set_target_properties(${MODULE_NAME} PROPERTIES
    BUILD_RPATH "@loader_path;@executable_path/../Frameworks"
    INSTALL_RPATH "@loader_path;@executable_path/../Frameworks"
)
