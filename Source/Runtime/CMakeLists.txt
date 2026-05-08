set(MODULE_NAME Launch)

set(MODULE_SRC
	Launch.cpp
	Application.h
	Application.cpp
)

add_executable(${MODULE_NAME}
	${MODULE_SRC}
)

set_target_properties(${MODULE_NAME} PROPERTIES FOLDER ${CZ_RUNTIME_MODULE_FOLDER})

target_link_libraries(${MODULE_NAME} PRIVATE
	CZCoreLibs
)

target_link_options(${MODULE_NAME} PRIVATE -rdynamic)

target_include_directories(${MODULE_NAME} PRIVATE
    ${CHOZO_SOURCE_DIR}
    ${CHOZO_INCLUDE_DIR}
)
