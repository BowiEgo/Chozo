set(MODULE_NAME CZProfile)

set(MODULE_TYPE Core)

set(MODULE_INCLUDE
    ${CHOZO_INCLUDE_DIR}/Core/Profile/TracyProfile.h
)

set(MODULE_SRC
    TracyProfile.cpp
)

add_chozo_module(
    NAME    ${MODULE_NAME}
    TYPE    ${MODULE_TYPE}
    INCLUDE ${MODULE_INCLUDE}
    SOURCE  ${MODULE_SRC}
    LINK    TracyClient
)

add_chozo_module_tests()
