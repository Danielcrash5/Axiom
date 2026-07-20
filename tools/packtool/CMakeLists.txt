add_executable(AxiomPackTool PackTool.cpp)

target_sources(AxiomPackTool PRIVATE
    ${CMAKE_SOURCE_DIR}/axiom/src/assets/VFS.cpp
)

target_include_directories(AxiomPackTool PRIVATE
    ${CMAKE_SOURCE_DIR}/axiom/include
)

target_link_libraries(AxiomPackTool PRIVATE minizip)
