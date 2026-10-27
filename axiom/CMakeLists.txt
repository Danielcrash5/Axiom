add_library(axiom_engine STATIC)

# Alle Quellen automatisch einbinden
file(GLOB_RECURSE AXIOM_SOURCES
    CONFIGURE_DEPENDS
    src/*.cpp
)

set(AXIOM_TEST_SOURCES ${AXIOM_SOURCES})

list(FILTER AXIOM_TEST_SOURCES EXCLUDE REGEX "/EntryPoint\\.cpp$")

# Vulkan
find_package(Vulkan REQUIRED)

# shaderc_combined: fuer ShaderCompiler (GLSL->SPIR-V zur Laufzeit, ersetzt
# tools/bake_shaders.py). NICHT die vom Vulkan-SDK mitgelieferte
# shaderc_combined(d).lib verwenden - die ist MSVC-kompiliert (Name-Mangling
# + MSVC-STL-ABI), komplett inkompatibel mit unserem MinGW/libstdc++-
# Toolchain (kein Linker-Konfigurationsfehler, sondern ein fundamentaler
# ABI-Konflikt - keine Kombination von CMake-Flags behebt das). Deshalb
# wird shaderc selbst aus dem Quellcode vendort und mit unserem eigenen
# Compiler gebaut ("Build with MinGW" ist ein offiziell unterstuetzter
# shaderc-Build-Pfad, kein Hack). SHADERC_SKIP_TESTS/EXAMPLES halten den
# Build so klein wie moeglich - trotzdem deutlich schwerer als die
# anderen vendorten Bibliotheken (zieht glslang + SPIRV-Tools +
# SPIRV-Headers mit), erster Configure/Build-Durchlauf dauert spuerbar
# laenger.
set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_COPYRIGHT_CHECK ON CACHE BOOL "" FORCE)
set(SPIRV_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
set(SPIRV_WERROR OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
# ACHTUNG: SHADERC_SKIP_INSTALL/ENABLE_SPIRV_TOOLS_INSTALL/
# SKIP_GLSLANG_INSTALL NICHT setzen - das bricht beim Configure mit einem
# Export-Set-Fehler (glslangs install(EXPORT) verweist dann auf ein
# SPIRV-Tools-opt-Target ohne eigenes Export-Set). Erprobt: Configure
# funktioniert nur OHNE diese drei Flags.
add_subdirectory(
    ${CMAKE_SOURCE_DIR}/thirdparty/shaderc
    ${CMAKE_BINARY_DIR}/thirdparty/shaderc
)

# Include Pfade
target_include_directories(axiom_engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/thirdparty/glm
)

# Compiler-Definitionen
target_compile_definitions(axiom_engine PUBLIC
    AXIOM_ENGINE_ASSET_PATH="${AXIOM_ENGINE_ASSET_PATH}"
    GLM_ENABLE_EXPERIMENTAL
)

if(AXIOM_ENABLE_CONSOLE_LOG)
    target_compile_definitions(axiom_engine PUBLIC
        AXIOM_ENABLE_CONSOLE_LOG
    )
endif()

# Thirdparty / Dependencies
target_link_libraries(axiom_engine
    PUBLIC
        Vulkan::Vulkan
        shaderc_combined
        minizip
        SDL3::SDL3
        stb
        imgui
        nlohmann_json::nlohmann_json
        cereal::cereal
        flecs::flecs
)

target_sources(axiom_engine PRIVATE
    ${AXIOM_SOURCES}
)

# Precompiled Header - siehe include/axiom/pch.h fuer die Regeln, was rein
# darf (nur stabile Drittanbieter-Header, NIE vma/vk_mem_alloc.h). Muss
# NACH target_link_libraries() stehen, damit die Include-Pfade der
# verlinkten Ziele (imgui, SDL3, flecs, ...) beim Erzeugen der PCH schon
# bekannt sind.
target_precompile_headers(axiom_engine PRIVATE include/axiom/pch.h)


# ============================================================
# Test Engine
# ============================================================

add_library(axiom_engine_test STATIC)

target_sources(axiom_engine_test PRIVATE
    ${AXIOM_TEST_SOURCES}
)

target_include_directories(axiom_engine_test
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/thirdparty/glm
)

target_compile_definitions(axiom_engine_test PUBLIC
    AXIOM_ENGINE_ASSET_PATH="${AXIOM_ENGINE_ASSET_PATH}"
    GLM_ENABLE_EXPERIMENTAL
)

if(AXIOM_ENABLE_CONSOLE_LOG)
    target_compile_definitions(axiom_engine_test PUBLIC
        AXIOM_ENABLE_CONSOLE_LOG
    )
endif()

target_link_libraries(axiom_engine_test
    PUBLIC
        Vulkan::Vulkan
        shaderc_combined
        minizip
        SDL3::SDL3
        stb
        imgui
        nlohmann_json::nlohmann_json
        cereal::cereal
        flecs::flecs
)

# Gleiche PCH wie axiom_engine, siehe dort fuer die Begruendung/Regeln.
# Eigenes target_precompile_headers noetig - PCHs werden nicht zwischen
# Targets geteilt, auch nicht bei identischen Quellen/Flags.
target_precompile_headers(axiom_engine_test PRIVATE include/axiom/pch.h)