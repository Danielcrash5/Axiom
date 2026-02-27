add_executable(testbed
    main.cpp
)

target_link_libraries(testbed
    PRIVATE axiom_engine
)