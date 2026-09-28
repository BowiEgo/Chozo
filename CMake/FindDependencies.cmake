# Vulkan discovery for the runtime stack.
#
# The backend needs headers, the loader, and - on macOS - MoltenVK plus its ICD manifest so the
# editor can be shipped as a self-contained .app bundle. Instead of assuming a particular SDK
# directory layout (which differs between the LunarG SDK, Homebrew and Windows) everything is
# derived from the loader that `find_package(Vulkan)` located; the copy steps in
# Source/Launch/CMakeLists.txt then only use the variables defined here.
if(CZ_OPTION_BUILD_RUNTIME)
    find_package(Vulkan REQUIRED)

    message(STATUS "CHOZO Vulkan loader:              ${Vulkan_LIBRARY}")
    message(STATUS "CHOZO Vulkan include dir:         ${Vulkan_INCLUDE_DIR}")

    unset(CHOZO_VULKAN_LIB_DIR)
    unset(CHOZO_MOLTENVK_ICD_FILE)

    if(Vulkan_LIBRARY)
        # Directory holding libvulkan / libMoltenVK (LunarG SDK: <sdk>/lib, Homebrew: /opt/homebrew/lib).
        get_filename_component(CHOZO_VULKAN_LIB_DIR "${Vulkan_LIBRARY}" DIRECTORY)
    elseif(DEFINED ENV{VULKAN_SDK})
        set(CHOZO_VULKAN_LIB_DIR "$ENV{VULKAN_SDK}/lib")
    endif()

    if(CHOZO_VULKAN_LIB_DIR)
        # The ICD manifest lives next to the loader in some layouts and under share/vulkan in others.
        find_file(
            CHOZO_MOLTENVK_ICD_FILE
            NAMES MoltenVK_icd.json
            HINTS "${CHOZO_VULKAN_LIB_DIR}" "${CHOZO_VULKAN_LIB_DIR}/../share/vulkan/icd.d"
                  "$ENV{VULKAN_SDK}/share/vulkan/icd.d"
        )
        message(STATUS "CHOZO Vulkan lib dir:             ${CHOZO_VULKAN_LIB_DIR}")
        message(STATUS "CHOZO MoltenVK ICD:               ${CHOZO_MOLTENVK_ICD_FILE}")
    else()
        message(STATUS "CHOZO Vulkan lib dir:             (not derived from the loader)")
    endif()
endif()
