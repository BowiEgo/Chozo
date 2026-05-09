set(MODULE_NAME CZDynamicLibrary)

set(MODULE_TYPE Core)

set(MODULE_INCLUDE
	${CHOZO_INCLUDE_DIR}/Core/DynamicLibrary/DynamicLibrary.h
	${CHOZO_INCLUDE_DIR}/Core/DynamicLibrary/BackendRegistry.h
)

set(MODULE_SRC
	BackendRegistry.cpp
)

add_chozo_module(
    NAME    ${MODULE_NAME}
    TYPE    ${MODULE_TYPE}
    INCLUDE ${MODULE_INCLUDE}
    SOURCE  ${MODULE_SRC}
)

add_chozo_module_tests()
