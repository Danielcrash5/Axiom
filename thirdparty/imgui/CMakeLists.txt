# 1. ImGui Core- und OpenGL/SDL3-Backend-Dateien definieren
add_library(imgui STATIC
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    backends/imgui_impl_sdl3.cpp
    backends/imgui_impl_opengl3.cpp
)

find_package(OpenGL REQUIRED)

# 2. Include-Pfade direkt an das Target heften (PUBLIC bedeutet: Weitervererben)
target_include_directories(imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/backends
)

target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)

# 3. Abhängigkeiten an ImGui linken
target_link_libraries(imgui PUBLIC
    SDL3::SDL3
    OpenGL::GL
)
