set(MODULE_NAME CZPlatform)

set(MODULE_TYPE Core)

set(MODULE_INCLUDE
    ${CHOZO_INCLUDE_DIR}/Core/Platform/Platform.h
)

if(WIN32)
    message("//----------------- Platform: Windows -----------------//")
    set(platform_definitions CZ_PLATFORM_WINDOWS)
    set(MODULE_SRC
    )
elseif(UNIX)
    if(APPLE)
        message("//----------------- Platform: MacOS -----------------//")
        set(platform_definitions CZ_PLATFORM_MACOS)
        set(MODULE_SRC
            Mac/MacUtils.mm
        )
    else()
        message("//----------------- Platform: Linux -----------------//")
        set(platform_definitions CZ_PLATFORM_LINUX)
    endif()
else()
    message("//----------------- Platform: Unsupported -----------------//")
    set(platform_definitions CZ_PLATFORM_UNSUPPORTED)
endif()

add_chozo_module(
    NAME    ${MODULE_NAME}
    TYPE    ${MODULE_TYPE}
    INCLUDE ${MODULE_INCLUDE}
    SOURCE  ${MODULE_SRC}
)

if(WIN32)
    target_link_libraries(${MODULE_NAME} PRIVATE
        advapi32 shell32 user32 gdi32 ole32
    )
elseif(APPLE)
    target_link_libraries(${MODULE_NAME} PRIVATE
        "-framework AppKit"
        "-framework QuartzCore"
        "-framework QuickLookThumbnailing"
        "-framework UniformTypeIdentifiers"
    )
elseif(UNIX AND NOT APPLE)
    # target_link_libraries(${MODULE_NAME} PRIVATE dl pthread)
endif()


add_chozo_module_tests()
