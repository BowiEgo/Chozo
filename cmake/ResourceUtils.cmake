# ========== Resource Copy Utilities ==========

function(define_target_resource_path TARGET MACRO_NAME RELATIVE_PATH)
    target_compile_definitions(${TARGET} PRIVATE ${MACRO_NAME}="${RELATIVE_PATH}")
endfunction()

function(copy_file_to_target TARGET FROM_FILE TO_NAME)
    add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${FROM_FILE}
            $<TARGET_FILE_DIR:${TARGET}>/${TO_NAME}
            COMMENT "Copying file ${TO_NAME} to target output dir"
    )
endfunction()

function(copy_file_to_parent TARGET FROM_FILE TO_NAME)
    add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${FROM_FILE}
            $<TARGET_FILE_DIR:${TARGET}>/../${TO_NAME}
            COMMENT "Copying file ${TO_NAME} to target output parent dir"
    )
endfunction()

function(copy_directory_to_target TARGET FROM_DIR TO_NAME)
    add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${FROM_DIR}
            $<TARGET_FILE_DIR:${TARGET}>/${TO_NAME}
            COMMENT "Copying directory ${TO_NAME} to target output dir"
    )
endfunction()

function(copy_directory_to_parent TARGET FROM_DIR TO_NAME)
    add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${FROM_DIR}
            $<TARGET_FILE_DIR:${TARGET}>/../${TO_NAME}
            COMMENT "Copying directory ${TO_NAME} to target output parent dir"
    )
endfunction()

# Usage:
#   create_directories(
#       TARGET MyExe
#       DIRS
#           assets/HDRIs
#           assets/Materials
#           assets/Meshes
#           assets/Scenes
#           assets/Textures
#   )
function(create_directories)
    cmake_parse_arguments(CD "" "TARGET" "DIRS" ${ARGN})

    if(NOT CD_TARGET OR NOT CD_DIRS)
        message(FATAL_ERROR "create_directories requires TARGET and DIRS")
    endif()

    foreach(dir IN LISTS CD_DIRS)
        add_custom_command(TARGET ${CD_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                    "$<TARGET_FILE_DIR:${CD_TARGET}>/${dir}"
            COMMENT "Creating directory ${dir} in runtime output dir"
        )
    endforeach()
endfunction()

function(copy_platform_imgui_ini TARGET)
    if(WIN32)
        set(PLATFORM "Windows")
    elseif(APPLE)
        set(PLATFORM "Mac")
    else()
        message(WARNING "Unsupported platform for imgui.ini copy.")
        return()
    endif()

    set(SOURCE_PATH "${CMAKE_SOURCE_DIR}/ChozoEditor/settings/${PLATFORM}/imgui.ini")

    add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${SOURCE_PATH}
            $<TARGET_FILE_DIR:${TARGET}>/imgui.ini
            COMMENT "Copying imgui.ini for platform: ${PLATFORM}"
    )
endfunction()

message(STATUS "[ResourceUtils] Resource copy functions loaded.")