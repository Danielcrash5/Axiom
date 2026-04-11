cmake_minimum_required(VERSION 3.25)
project(Axiom LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(AXIOM_ENABLE_CONSOLE_LOG "Enable console logging" ON)

# Engine asset path
set(AXIOM_ENGINE_ASSET_PATH "${CMAKE_SOURCE_DIR}/axiom/assets/" CACHE STRING "")

# Game asset path
set(AXIOM_GAME_ASSET_PATH "${CMAKE_SOURCE_DIR}/testbed/assets/" CACHE STRING "")

# Engine Projects
add_subdirectory(axiom)
# Thirdparty Libraries

# GLFW Options
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

set(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/thirdparty)


# Diese Defines an den Compiler übergeben
add_compile_definitions(
    AXIOM_ENGINE_ASSET_PATH="${AXIOM_ENGINE_ASSET_PATH}"
    AXIOM_GAME_ASSET_PATH="${AXIOM_GAME_ASSET_PATH}"
)


add_subdirectory(thirdparty/glfw)

# GLAD - OpenGL Loader
add_subdirectory(thirdparty/glad)

add_subdirectory(thirdparty/stb)

set(MZ_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(MZ_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(MZ_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(MZ_ZLIB OFF CACHE BOOL "" FORCE)
set(MZ_BZIP2 OFF CACHE BOOL "" FORCE)
set(MZ_LZMA OFF CACHE BOOL "" FORCE)
set(MZ_ZSTD OFF CACHE BOOL "" FORCE)
set(MZ_OPENSSL OFF CACHE BOOL "" FORCE)
set(MZ_LIBCOMP OFF CACHE BOOL "" FORCE)

add_subdirectory(thirdparty/minizip-ng)

add_subdirectory(tools/packtool)

add_subdirectory(thirdparty/entt)


add_subdirectory(testbed)
