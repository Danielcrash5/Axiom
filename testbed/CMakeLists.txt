file(GLOB_RECURSE TESTBED_SOURCES
    CONFIGURE_DEPENDS
    src/*.cpp
)

add_executable(testbed)

target_include_directories(testbed
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_sources(testbed PRIVATE ${TESTBED_SOURCES})

# Link Engine + Slang statisch
target_link_libraries(testbed
    PRIVATE axiom_engine
)


# Output-Verzeichnis und Debug-Verzeichnis
set(TESTBED_RUNTIME_DIR "${CMAKE_BINARY_DIR}/testbed")
set(TESTBED_GAME_ASSET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")

set_target_properties(testbed PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${TESTBED_RUNTIME_DIR}"
    VS_DEBUGGER_WORKING_DIRECTORY "${TESTBED_RUNTIME_DIR}"
)

target_compile_definitions(testbed PRIVATE
    AXIOM_GAME_ASSET_PATH="${TESTBED_GAME_ASSET_DIR}/"
)

axiom_add_asset_pack(testbed_engine_assets "${TESTBED_RUNTIME_DIR}" "${AXIOM_ENGINE_PACK_NAME}" "${AXIOM_ENGINE_ASSET_DIR}")
add_dependencies(testbed testbed_engine_assets)

# Optional: Win32 Executable ohne Console
if(WIN32 AND NOT AXIOM_ENABLE_CONSOLE_LOG)
    set_target_properties(testbed PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
