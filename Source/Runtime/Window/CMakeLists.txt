set(MODULE_NAME CZWindow)
set(MODULE_TYPE RUNTIME)

set(MODULE_INCLUDE
    ${CHOZO_INCLUDE_DIR}/Runtime/Window/Window.hpp
    SDLWindow/SDLWindowObj.hpp
    SDLWindow/SDLInputImpl.hpp
)

set(MODULE_SRC
    Window.cpp
    SDLWindow/SDLWindowObj.cpp
)

add_chozo_module(
    NAME    ${MODULE_NAME}
    TYPE    ${MODULE_TYPE}
    INCLUDE ${MODULE_INCLUDE}
    SOURCE  ${MODULE_SRC}
    LINK    SDL3::SDL3 CZCoreLibs
)

add_chozo_module_tests()