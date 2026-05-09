set(MODULE_NAME CZTest)

set(MODULE_SRC Main.cpp)

add_executable(${MODULE_NAME}
    ${MODULE_SRC}
    ${ALL_TEST_FILES}
)

set_target_properties(${MODULE_NAME} PROPERTIES FOLDER "Tests")

target_link_libraries(${MODULE_NAME} PRIVATE
    CZCoreLibs 
    doctest::doctest
)

target_link_options(${MODULE_NAME} PRIVATE -rdynamic)

target_include_directories(${MODULE_NAME} PRIVATE
    ${CHOZO_SOURCE_DIR}
    ${CHOZO_INCLUDE_DIR}
)