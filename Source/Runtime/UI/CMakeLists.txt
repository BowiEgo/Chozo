set(MODULE_NAME CZUI)

set(MODULE_TYPE RUNTIME)

# Interface between the editor UI and a graphics backend (plus the factory that asks the loaded
# backend module for its implementation).
set(MODULE_INCLUDE
    ${CHOZO_INCLUDE_DIR}/Runtime/UI/UIRenderBackend.hpp
)

set(MODULE_SRC
    UIRenderBackend.cpp
    Perf/FrameStats.cpp
    Perf/PerfOverlay.cpp
)

add_chozo_module(
    NAME    ${MODULE_NAME}
    TYPE    ${MODULE_TYPE}
    INCLUDE ${MODULE_INCLUDE}
    SOURCE  ${MODULE_SRC}
    LINK    CZCoreLibs CZRHI CZWindow
)
