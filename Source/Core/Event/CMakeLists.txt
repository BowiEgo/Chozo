set(MODULE_NAME CZEvent)

set(MODULE_TYPE Core)

set(MODULE_INCLUDE
	${CHOZO_INCLUDE_DIR}/Core/Event/Event.hpp
	${CHOZO_INCLUDE_DIR}/Core/Event/Input.hpp
	${CHOZO_INCLUDE_DIR}/Core/Event/KeyCodes.hpp
)

set(MODULE_SRC
	Event.cpp
	Input.cpp
)

add_chozo_module(
    NAME    ${MODULE_NAME}
    TYPE    ${MODULE_TYPE}
    INCLUDE ${MODULE_INCLUDE}
    SOURCE  ${MODULE_SRC}
)

add_chozo_module_tests()
