add_executable(testbed
    main.cpp
)

# Link Engine + Slang statisch
target_link_libraries(testbed
    PRIVATE axiom_engine
    PRIVATE ${SLANG_LIB}
)

# Include Slang Header
target_include_directories(testbed PRIVATE
    ${SLANG_INCLUDE_DIR}
)

# Output-Verzeichnis und Debug-Verzeichnis
set_target_properties(testbed PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/testbed"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/testbed"
)

# Assets kopieren
file(COPY "${CMAKE_SOURCE_DIR}/assets" DESTINATION "${CMAKE_BINARY_DIR}/testbed")

# Slang DLLs kopieren
add_custom_command(TARGET testbed POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SLANG_BIN}/slang.dll
        $<TARGET_FILE_DIR:testbed>
)

add_custom_command(TARGET testbed POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SLANG_BIN}/slang-compiler.dll
        $<TARGET_FILE_DIR:testbed>
)

# Optional: Win32 Executable ohne Console
if(WIN32 AND NOT AXIOM_ENABLE_CONSOLE_LOG)
    set_target_properties(testbed PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()