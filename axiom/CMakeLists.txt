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

# Include Pfade
target_include_directories(axiom_engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/thirdparty/glm
    PRIVATE
        ${CMAKE_SOURCE_DIR}/thirdparty/vma
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
        minizip
        SDL3::SDL3
        stb
        imgui
        nlohmann_json::nlohmann_json
        cereal::cereal
        EnTT::EnTT
)

target_sources(axiom_engine PRIVATE
    ${AXIOM_SOURCES}
)


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
    PRIVATE
        ${CMAKE_SOURCE_DIR}/thirdparty/vma
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
        minizip
        SDL3::SDL3
        stb
        imgui
        nlohmann_json::nlohmann_json
        cereal::cereal
        EnTT::EnTT
)