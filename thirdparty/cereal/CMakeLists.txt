add_library(cereal INTERFACE)
add_library(cereal::cereal ALIAS cereal)

target_include_directories(cereal INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
