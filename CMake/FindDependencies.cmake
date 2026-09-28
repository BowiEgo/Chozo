# Find Vulkan (only needed by the runtime stack; SPIRV-Tools is not used yet)
if(CZ_OPTION_BUILD_RUNTIME)
    find_package(Vulkan REQUIRED)
endif()

set(CHOZO_VULKAN_INCLUDE_DIR ${Vulkan_INCLUDE_DIR})
set(CHOZO_VULKAN_LIB_DIR     ${Vulkan_INCLUDE_DIR}/../Lib)
set(CHOZO_VULKAN_ICD_DIR     ${Vulkan_INCLUDE_DIR}/../share/vulkan/icd.d)

message(STATUS "CHOZO Vulkan INCLUDE DIR:  ${CHOZO_VULKAN_INCLUDE_DIR}")
message(STATUS "CHOZO Vulkan LIB DIR:      ${CHOZO_VULKAN_LIB_DIR}")
message(STATUS "CHOZO Vulkan ICD DIR:      ${CHOZO_VULKAN_ICD_DIR}")