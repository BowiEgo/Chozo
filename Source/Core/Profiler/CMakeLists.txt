set(MODULE_NAME CZProfiler)

set(MODULE_TYPE Core)

# Header-only module: the Tracy instrumentation macros live in TracyProfiler.h, so there is
# nothing to compile. A normal static library would be empty, which makes ranlib complain about
# a missing table of contents.
add_library(${MODULE_NAME} INTERFACE)
target_include_directories(${MODULE_NAME} INTERFACE ${CHOZO_INCLUDE_DIR})
target_link_libraries(${MODULE_NAME} INTERFACE Tracy::TracyClient)
set_target_properties(${MODULE_NAME} PROPERTIES FOLDER ${CZ_CORE_MODULE_FOLDER})

# Register with the list consumed by CZCoreLibs (mirrors add_chozo_module).
set(CZ_CORE_MODULE_LIST "${CZ_CORE_MODULE_LIST};${MODULE_NAME}" CACHE STRING "" FORCE)
