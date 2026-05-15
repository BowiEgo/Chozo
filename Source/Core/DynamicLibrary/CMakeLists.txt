set(MODULE_NAME CZDynamicLibrary)

set(MODULE_TYPE Core)

set(MODULE_INCLUDE
	${CHOZO_INCLUDE_DIR}/Core/DynamicLibrary/DynamicLibrary.hpp
	${CHOZO_INCLUDE_DIR}/Core/DynamicLibrary/DynamicLibraryRegistry.hpp
)

set(MODULE_SRC
	DynamicLibraryRegistry.cpp
)

add_chozo_module(
    NAME    ${MODULE_NAME}
    TYPE    ${MODULE_TYPE}
    INCLUDE ${MODULE_INCLUDE}
    SOURCE  ${MODULE_SRC}
)

add_chozo_module_tests()
