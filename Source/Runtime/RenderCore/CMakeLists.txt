set(MODULE_NAME CZRenderCore)

set(MODULE_INCLUDE
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/Renderer.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/Viewport.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/Asset.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/Shader.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/ShaderCompiler.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/AssetManager.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/AssetRegistry.hpp
    ${CHOZO_INCLUDE_DIR}/Runtime/RenderCore/ShaderRegistry.hpp
)

set(MODULE_SRC
    Renderer.cpp
    Viewport.cpp
    ShaderUtils.cpp
    ShaderCompiler.cpp
    ShaderObj.cpp
    AssetManager.cpp
    ShaderRegistry.cpp
)

add_chozo_module(
    NAME    ${MODULE_NAME}
    TYPE    RUNTIME
    INCLUDE ${MODULE_INCLUDE}
    SOURCE  ${MODULE_SRC}
    LINK    CZCoreLibs slang::slang
)

add_chozo_module_tests()