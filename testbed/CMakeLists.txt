add_executable(testbed
    main.cpp
)

target_link_libraries(testbed
    PRIVATE axiom_engine
)

set_target_properties(testbed PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/testbed"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/testbed"
)

file(COPY "${CMAKE_SOURCE_DIR}/assets" DESTINATION "${CMAKE_BINARY_DIR}/testbed")

if(WIN32)
    if(NOT AXIOM_ENABLE_CONSOLE_LOG)
        set_target_properties(testbed PROPERTIES
            WIN32_EXECUTABLE TRUE
        )
    endif()
endif()