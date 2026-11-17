add_executable(AxiomPackTool PackTool.cpp)

target_sources(AxiomPackTool PRIVATE
    ${CMAKE_SOURCE_DIR}/axiom/src/assets/VFS.cpp
)
