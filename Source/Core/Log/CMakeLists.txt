set(MODULE_NAME CZLog)

set(MODULE_TYPE Core)

set(MODULE_INCLUDE
	${CHOZO_INCLUDE_DIR}/Core/Log/Logger.h
	${CHOZO_INCLUDE_DIR}/Core/Log/LogMacros.h
)

set(MODULE_SRC
	Logger.cpp
)

add_chozo_module(
    NAME    ${MODULE_NAME}
    TYPE    ${MODULE_TYPE}
    INCLUDE ${MODULE_INCLUDE}
    SOURCE  ${MODULE_SRC}
    LINK    spdlog::spdlog_header_only
)

add_chozo_module_tests()
