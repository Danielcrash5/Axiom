cmake_minimum_required(VERSION 3.20)
project(flecs LANGUAGES C CXX)

# Amalgamierte Single-File-Distribution (distr/flecs.c + distr/flecs.h aus
# dem offiziellen Repo), analog zu entt/single_include - nur dass flecs
# eine C-Bibliothek ist und daher tatsaechlich kompiliert werden muss.
add_library(flecs STATIC src/flecs.c)
add_library(flecs::flecs ALIAS flecs)

target_include_directories(flecs PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Kein FLECS_CUSTOM_BUILD definieren - ohne dieses Macro sind in der
# amalgamierten Distribution ALLE Addons (inkl. der C++-API) standardmaessig
# aktiv. Mit FLECS_CUSTOM_BUILD muesste man jedes gewuenschte Addon einzeln
# wieder aktivieren - fuer den ersten Wurf unnoetig.

if (WIN32)
    target_link_libraries(flecs PUBLIC
        ws2_32
        dbghelp
    )
endif()
