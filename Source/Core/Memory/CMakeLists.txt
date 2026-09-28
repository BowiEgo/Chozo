set(MODULE_NAME CZMemory)

set(MODULE_TYPE Core)

# Shared: the allocation accounting and leak tracker must exist exactly once per process,
# otherwise allocations made in a plugin/backend dylib are freed by a different copy of the
# state (see docs/TODO.md, item P1-1).
set(MODULE_LIBRARY_TYPE SHARED)

set(MODULE_INCLUDE
    ${CHOZO_INCLUDE_DIR}/Core/Memory/Memory.hpp
    ${CHOZO_INCLUDE_DIR}/Core/Memory/Allocator.hpp
    ${CHOZO_INCLUDE_DIR}/Core/Memory/TypeMemoryTracker.hpp
)

set(MODULE_SRC
    Memory.cpp
    Allocator.cpp
    TypeMemoryTracker.cpp
)

add_chozo_module(
    NAME         ${MODULE_NAME}
    TYPE         ${MODULE_TYPE}
    LIBRARY_TYPE ${MODULE_LIBRARY_TYPE}
    INCLUDE      ${MODULE_INCLUDE}
    SOURCE       ${MODULE_SRC}
    LINK         CZLog
)

add_chozo_module_tests()
