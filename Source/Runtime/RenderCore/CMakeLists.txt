set(MODULE_NAME CZRenderCore)

set(MODULE_INCLUDE
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/Renderer.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/Viewport.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/Asset.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/Shader.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/ShaderCompiler.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/AssetRegistry.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/ShaderRegistry.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/Params.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/MeshParams.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/Mesh.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/ProceduralMesh/ProceduralMesh.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/Scene/Entity.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/Scene/Scene.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/Scene/TransformSystem.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/Camera/Camera.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/Camera/CameraManager.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/Camera/SceneCamera.hpp
)

set(MODULE_SRC
    Renderer.cpp
    Viewport.cpp
    ShaderUtils.cpp
    ShaderCompiler.cpp
    ShaderObj.cpp
    ShaderRegistry.cpp
    Mesh.cpp
    MeshRegistry.cpp
    ProceduralMesh/ProceduralMesh.cpp
    ProceduralMesh/CubeParamsObj.cpp
    Components/TransformComponent.cpp
    Scene/Scene.cpp
    Scene/TransformSystem.cpp
    Camera/Camera.cpp
    Camera/CameraManager.cpp
)

add_chozo_module(
    NAME    ${MODULE_NAME}
    TYPE    RUNTIME
    INCLUDE ${MODULE_INCLUDE}
    SOURCE  ${MODULE_SRC}
    LINK    CZCoreLibs CZRHI slang::slang EnTT::EnTT
)

# RenderCore tests link the module (and therefore Slang) but need no GPU: they cover the
# scene/ECS, transform and parameter layers. They get their own executable so that the core-only
# CZTest stays free of runtime dependencies.
file(GLOB RENDERCORE_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Tests/*Test.cpp")

if(RENDERCORE_TEST_SOURCES)
    add_executable(CZRenderCoreTests Tests/Main.cpp ${RENDERCORE_TEST_SOURCES})
    set_target_properties(CZRenderCoreTests PROPERTIES FOLDER "Tests")
    target_link_libraries(CZRenderCoreTests PRIVATE CZRenderCore CZRHI CZCoreLibs doctest::doctest)
    target_include_directories(CZRenderCoreTests PRIVATE ${CHOZO_SOURCE_DIR} ${CHOZO_INCLUDE_DIR})
    add_test(NAME CZRenderCoreTests COMMAND CZRenderCoreTests)
endif()