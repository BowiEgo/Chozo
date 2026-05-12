set(MODULE_NAME CZLayer)

set(MODULE_TYPE Core)

set(MODULE_INCLUDE
	${CHOZO_INCLUDE_DIR}/Core/Layer/Layer.h
	${CHOZO_INCLUDE_DIR}/Core/Layer/LayerStack.h
)

set(MODULE_SRC
    LayerStack.cpp
)

add_chozo_module(
    NAME    ${MODULE_NAME}
    TYPE    ${MODULE_TYPE}
    INCLUDE ${MODULE_INCLUDE}
    SOURCE  ${MODULE_SRC}
)

add_chozo_module_tests()
