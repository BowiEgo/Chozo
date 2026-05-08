include(FetchContent)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 10.2.1
)
FetchContent_MakeAvailable(fmt)

## Fetch spdlog

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.17.0
)

FetchContent_MakeAvailable(spdlog)
message(STATUS "CHOZO SPDLOG SOURCE DIR:     ${spdlog_SOURCE_DIR}")

## Fetch SDL3

FetchContent_Declare(
  sdl3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-3.4.8
)

set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_DISABLE_INSTALL_DOCS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(sdl3)
message(STATUS "CHOZO SDL3 SOURCE DIR:     ${sdl3_SOURCE_DIR}")