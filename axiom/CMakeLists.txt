add_library(axiom_engine STATIC)

# Alle Quellen automatisch einbinden
file(GLOB_RECURSE AXIOM_SOURCES
    CONFIGURE_DEPENDS
    src/*.cpp
)

target_link_libraries(axiom_engine PUBLIC minizip)

target_compile_definitions(axiom_engine PUBLIC
    AXIOM_ENGINE_ASSET_PATH="${CMAKE_SOURCE_DIR}/axiom/assets/"
    AXIOM_GAME_ASSET_PATH="${CMAKE_SOURCE_DIR}/testbed/assets/"
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
target_compile_definitions(axiom_engine PUBLIC GLFW_INCLUDE_NONE GLM_ENABLE_EXPERIMENTAL)

# OpenGL & Thirdparty
find_package(OpenGL REQUIRED)
target_link_libraries(axiom_engine
    PUBLIC glfw OpenGL::GL glad stb
)

target_link_libraries(axiom_engine PUBLIC EnTT::EnTT)