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
# tools/bake_shaders.py). Teil jedes Vulkan-SDK-Installs, kein zusaetzliches
# Tooling/Download noetig. Vulkan::shaderc_combined wird von CMakes
# FindVulkan-Modul (>= 3.24) bereitgestellt, wenn die Komponente gefunden
# wird - deshalb hier separat angefragt statt vorauszusetzen, dass
# find_package(Vulkan REQUIRED) es automatisch mitbringt.
find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)

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
        Vulkan::shaderc_combined
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
        Vulkan::shaderc_combined
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