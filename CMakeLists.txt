cmake_minimum_required(VERSION 3.25)
project(Axiom LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(AXIOM_ENABLE_CONSOLE_LOG "Enable console logging" ON)

set(AXIOM_ENGINE_ASSET_DIR "${CMAKE_SOURCE_DIR}/axiom/assets" CACHE PATH "Engine asset directory")
set(AXIOM_ENGINE_PACK_NAME "engine.AxPack" CACHE STRING "Engine asset pack file name")
set(AXIOM_ENGINE_ASSET_PATH "./${AXIOM_ENGINE_PACK_NAME}" CACHE STRING "Engine asset path used by runtime" FORCE)

# Thirdparty Libraries

set(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/thirdparty)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/SDL)

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

add_subdirectory(thirdparty/imgui)

# Engine Projects
add_subdirectory(axiom)

function(axiom_add_asset_pack pack_target output_dir pack_name input_dir)
    if(NOT IS_DIRECTORY "${input_dir}")
        message(FATAL_ERROR "Asset pack input directory does not exist: ${input_dir}")
    endif()

    file(GLOB_RECURSE pack_input_files
        CONFIGURE_DEPENDS
        LIST_DIRECTORIES false
        "${input_dir}/*"
    )

    set(output_file "${output_dir}/${pack_name}")

    add_custom_command(
        OUTPUT "${output_file}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${output_dir}"
        COMMAND ${CMAKE_COMMAND} -E rm -f "${output_file}"
        COMMAND $<TARGET_FILE:AxiomPackTool> "${output_file}" "${input_dir}"
        DEPENDS AxiomPackTool ${pack_input_files}
        COMMENT "Packing ${pack_name}"
        VERBATIM
    )

    add_custom_target(${pack_target} DEPENDS "${output_file}")
endfunction()

add_subdirectory(testbed)
