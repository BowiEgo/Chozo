set(CZ_EMBED_RUNTIME_H "${CHOZO_BUILD_EMBED_DIR}/EmbedRuntime.h")
set(CZ_EMBED_RUNTIME_CPP "${CHOZO_BUILD_EMBED_DIR}/EmbedRuntime.cpp")

set_source_files_properties(
    "${CZ_EMBED_RUNTIME_H}"
    "${CZ_EMBED_RUNTIME_CPP}"
    PROPERTIES GENERATED TRUE)

if (CZ_OPTION_EMBED_RUNTIME)
    add_custom_command(
        OUTPUT "${CZ_EMBED_RUNTIME_H}" "${CZ_EMBED_RUNTIME_CPP}"
        COMMAND ${Python3_EXECUTABLE} -u "${CHOZO_SCRIPTS_DIR}/Embed.py" "$<TARGET_FILE:Launch>" "EmbedRuntime" "${CHOZO_BUILD_EMBED_DIR}"
        DEPENDS Launch
    )
else()
    add_custom_command(
        OUTPUT "${CZ_EMBED_RUNTIME_H}" "${CZ_EMBED_RUNTIME_CPP}"
        COMMAND ${CMAKE_COMMAND} -E echo "#pragma once" > "${CZ_EMBED_RUNTIME_H}"
        COMMAND ${CMAKE_COMMAND} -E echo "#include <cstddef>" >> "${CZ_EMBED_RUNTIME_H}"
        COMMAND ${CMAKE_COMMAND} -E echo "extern \"C\" { extern const unsigned char* EmbedRuntimeData; extern size_t EmbedRuntimeSize; }" >> "${CZ_EMBED_RUNTIME_H}"
        COMMAND ${CMAKE_COMMAND} -E echo "#include <cstddef>" > "${CZ_EMBED_RUNTIME_CPP}"
        COMMAND ${CMAKE_COMMAND} -E echo "extern \"C\" { const unsigned char* EmbedRuntimeData = nullptr; size_t EmbedRuntimeSize = 0; }" >> "${CZ_EMBED_RUNTIME_CPP}"
        VERBATIM
    )
endif()

add_custom_target(CZEmbedRuntime DEPENDS
    ${CZ_EMBED_RUNTIME_H}
    ${CZ_EMBED_RUNTIME_CPP}
)