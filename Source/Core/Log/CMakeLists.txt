set(MODULE_NAME CZLog)

set(MODULE_INCLUDE
	${CHOZO_INCLUDE_DIR}/Core/Log/Logger.h
)

set(MODULE_SRC
	Logger.cpp
)

add_chozo_core_module(
    NAME    ${MODULE_NAME}
    INCLUDE ${MODULE_INCLUDE}
    SOURCE  ${MODULE_SRC}
)

target_link_libraries(${MODULE_NAME} PRIVATE
	spdlog::spdlog_header_only
)

add_chozo_module_tests()
