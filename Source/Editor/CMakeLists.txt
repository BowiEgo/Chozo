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
# This module is built by hand instead of through `add_chozo_module`, so the shared prerequisites
# (fmt, glm, Tracy) have to be linked explicitly: `EditorCamera` uses glm's GLM_GTX helpers.
target_link_libraries(${MODULE_NAME} PRIVATE
    CZEnginePrerequisites
    CZCoreLibs
    CZRuntimeLibs
    Chozo::ImGui
)

target_precompile_headers(${MODULE_NAME} PRIVATE
    EditorPCH.h
)

if(NOT WIN32)
    target_link_options(${MODULE_NAME} PRIVATE -rdynamic)
endif()

if(APPLE)
    set_target_properties(${MODULE_NAME} PROPERTIES
        BUILD_RPATH "@loader_path;@executable_path/../Frameworks"
        INSTALL_RPATH "@loader_path;@executable_path/../Frameworks"
    )
elseif(UNIX)
    set_target_properties(${MODULE_NAME} PROPERTIES
        BUILD_RPATH "$ORIGIN"
        INSTALL_RPATH "$ORIGIN"
    )
endif()
