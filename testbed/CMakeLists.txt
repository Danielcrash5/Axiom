add_executable(testbed
    main.cpp
)

# Link Engine + Slang statisch
target_link_libraries(testbed
    PRIVATE axiom_engine
)

# Output-Verzeichnis und Debug-Verzeichnis
set_target_properties(testbed PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/testbed"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/testbed"
)

# Assets kopieren
file(COPY "${CMAKE_SOURCE_DIR}/assets" DESTINATION "${CMAKE_BINARY_DIR}/testbed")

# Optional: Win32 Executable ohne Console
if(WIN32 AND NOT AXIOM_ENABLE_CONSOLE_LOG)
    set_target_properties(testbed PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()