cmake_minimum_required(VERSION 3.25)
project(Axiom LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(AXIOM_ENABLE_CONSOLE_LOG "Enable console logging" ON)

# Engine Projects
add_subdirectory(axiom)
add_subdirectory(testbed)

# Thirdparty Libraries

# GLFW Options
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(thirdparty/glfw)

# GLAD - OpenGL Loader
add_library(glad thirdparty/glad/src/glad.c)

target_include_directories(glad PUBLIC
    thirdpaarty/glad/include
)

target_link_libraries(your_target
    glad
    glfw
)
