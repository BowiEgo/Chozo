set(MODULE_NAME CZMemory)

set(MODULE_TYPE Core)

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
    NAME    ${MODULE_NAME}
    TYPE    ${MODULE_TYPE}
    INCLUDE ${MODULE_INCLUDE}
    SOURCE  ${MODULE_SRC}
)

add_chozo_module_tests()
