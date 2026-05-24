# 全局默认选项
if(MSVC)
    set(MODULE_DEFAULT_COMPILE_OPTIONS /W4 /wd4201)
    set(MODULE_DEFAULT_LINK_OPTIONS "/ignore:4099")
else()
    set(MODULE_DEFAULT_COMPILE_OPTIONS -Wall -Wextra)
    set(MODULE_DEFAULT_LINK_OPTIONS)
endif()

function(add_chozo_module)
    cmake_parse_arguments(
        MODULE
        ""                              # options
        "NAME;TYPE;LIBRARY_TYPE;PCH"    
        "INCLUDE;SOURCE;LINK;INCLUDE_DIRS;COMPILE_OPTIONS;LINK_OPTIONS"
        ${ARGN}
    )

    # 默认类型为 CORE
    if(NOT MODULE_TYPE)
        set(MODULE_TYPE "CORE")
    endif()

    if(NOT MODULE_LIBRARY_TYPE)
        set(MODULE_LIBRARY_TYPE "STATIC")
    endif()

    string(TOUPPER "${MODULE_TYPE}" TYPE_UPPER)
    set(MODULE_LIST_VAR   "CZ_${TYPE_UPPER}_MODULE_LIST")
    set(MODULE_FOLDER_VAR "CZ_${TYPE_UPPER}_MODULE_FOLDER")

    message(STATUS "CHOZO ${TYPE_UPPER} MODULE (${MODULE_LIBRARY_TYPE}) ${MODULE_NAME}")

    set(${MODULE_LIST_VAR} "${${MODULE_LIST_VAR}};${MODULE_NAME}" CACHE STRING "" FORCE)

    add_library(${MODULE_NAME} ${MODULE_LIBRARY_TYPE} ${MODULE_SOURCE})

    if(MODULE_LIBRARY_TYPE STREQUAL "SHARED")
        target_compile_definitions(${MODULE_NAME} PRIVATE ${MODULE_NAME}_EXPORTS)
    endif()

    if(MODULE_INCLUDE)
        target_sources(${MODULE_NAME} PRIVATE ${MODULE_INCLUDE})
    endif()

    target_include_directories(${MODULE_NAME} PRIVATE
        ${MODULE_INCLUDE_DIRS}
        ${CHOZO_INCLUDE_DIR}
    )

    if(MODULE_COMPILE_OPTIONS)
        target_compile_options(${MODULE_NAME} PRIVATE ${MODULE_COMPILE_OPTIONS})
    else()
        target_compile_options(${MODULE_NAME} PRIVATE ${MODULE_DEFAULT_COMPILE_OPTIONS})
    endif()

    if(MODULE_LINK_OPTIONS)
        target_link_options(${MODULE_NAME} PRIVATE ${MODULE_LINK_OPTIONS})
    else()
        if(MODULE_DEFAULT_LINK_OPTIONS)
            target_link_options(${MODULE_NAME} PRIVATE ${MODULE_DEFAULT_LINK_OPTIONS})
        endif()
    endif()

    if(MODULE_LINK)
        target_link_libraries(${MODULE_NAME} PRIVATE ${MODULE_LINK})
    endif()

    target_link_libraries(${MODULE_NAME} PRIVATE CZEnginePrerequisites)

    set_target_properties(${MODULE_NAME} PROPERTIES FOLDER ${${MODULE_FOLDER_VAR}})

    if(MODULE_PCH)
        target_precompile_headers(${MODULE_NAME} PRIVATE ${MODULE_PCH})
    endif()

    # add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
    #     COMMAND ${CMAKE_COMMAND} -E copy_if_different
    #         "$<TARGET_FILE:${MODULE_NAME}>"
    #         "$<TARGET_FILE_DIR:Launch>"
    # )
endfunction()