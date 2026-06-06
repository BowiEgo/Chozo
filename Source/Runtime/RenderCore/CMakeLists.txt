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
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/MeshParamsInternal.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/MeshParams.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/Mesh.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/ProceduralMesh/ProceduralMesh.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/Scene/Entity.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/Scene/Scene.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/Scene/TransformSystem.hpp
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
    Components/TransformParams.cpp
    Components/TransformComponent.cpp
    Scene/Entity.cpp
    Scene/Scene.cpp
    Scene/TransformSystem.cpp
)

add_chozo_module(
    NAME    ${MODULE_NAME}
    TYPE    RUNTIME
    INCLUDE ${MODULE_INCLUDE}
    SOURCE  ${MODULE_SRC}
    LINK    CZCoreLibs slang::slang EnTT::EnTT
)

add_chozo_module_tests()