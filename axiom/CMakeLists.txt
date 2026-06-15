add_library(axiom_engine STATIC)

# Alle Quellen automatisch einbinden
file(GLOB_RECURSE AXIOM_SOURCES
    CONFIGURE_DEPENDS
    src/*.cpp
)

target_link_libraries(axiom_engine PUBLIC minizip)

target_compile_definitions(axiom_engine PUBLIC
    AXIOM_ENGINE_ASSET_PATH="${AXIOM_ENGINE_ASSET_PATH}"
)


target_sources(axiom_engine PRIVATE ${AXIOM_SOURCES})

# Include Pfade
target_include_directories(axiom_engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/thirdparty/glm
)

# Compiler-Definitionen
if(AXIOM_ENABLE_CONSOLE_LOG)
    target_compile_definitions(axiom_engine PUBLIC AXIOM_ENABLE_CONSOLE_LOG)
endif()
target_compile_definitions(axiom_engine PUBLIC GLM_ENABLE_EXPERIMENTAL)

if(AXIOM_RENDERER_VULKAN)
    target_compile_definitions(
        axiom_engine
        PRIVATE
        AXIOM_HAS_VULKAN
    )
endif()

if(AXIOM_RENDERER_WEBGPU)
    target_compile_definitions(
        axiom_engine
        PRIVATE
        AXIOM_HAS_WEBGPU
    )
endif()

if(AXIOM_RENDERER_VULKAN)

    find_package(Vulkan REQUIRED COMPONENTS
        volk
        shaderc_combined
    )

    target_link_libraries(axiom_engine PRIVATE
        Vulkan::Vulkan
        Vulkan::volk
        Vulkan::shaderc_combined
    )

endif()

# Thirdparty
target_link_libraries(axiom_engine
    PUBLIC SDL3::SDL3 stb imgui
)

target_link_libraries(axiom_engine PUBLIC EnTT::EnTT)
