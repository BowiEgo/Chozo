set(MODULE_NAME CZTypeRegistry)

set(MODULE_TYPE Core)

set(MODULE_INCLUDE
    ${CHOZO_INCLUDE_DIR}/Core/TypeRegistry/TypeMask.hpp
    ${CHOZO_INCLUDE_DIR}/Core/TypeRegistry/TypeRegistry.hpp
)

set(MODULE_SRC
    TypeMask.cpp
    TypeRegistry.cpp
)

add_chozo_module(
    NAME    ${MODULE_NAME}
    TYPE    ${MODULE_TYPE}
    INCLUDE ${MODULE_INCLUDE}
    SOURCE  ${MODULE_SRC}
)

add_chozo_module_tests()
