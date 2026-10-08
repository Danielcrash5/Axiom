#pragma once
// Precompiled Header - NUR stabile, schwere Drittanbieter-Header. NIE
// axiom-eigene Header hier rein (auch nicht transitiv ueber einen
// axiom-Header) und NIEMALS vma/vk_mem_alloc.h (Single-Header mit
// VMA_IMPLEMENTATION-Guard-Muster - siehe VmaImpl.cpp; wuerde der PCH
// diese Datei einmal ohne das Makro einlesen, verhindert der Include-Guard,
// dass VmaImpl.cpp den Implementierungsteil je zu Gesicht bekommt ->
// kompiliert fehlerfrei, linkt mit "undefined reference to vmaCreate*").
//
// Kandidat fuer die PCH ist ein Header nur, wenn er (a) schwer zu parsen
// ist UND (b) sich praktisch nie aendert. Aendert sich einer der Header
// hier, wird die PCH ungueltig und JEDE TU von axiom_engine baut neu -
// deshalb bewusst kein einziger axiom/-eigener Header, egal wie stabil er
// gerade wirkt.

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/quaternion.hpp>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

#include <flecs.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>

#include <nlohmann/json.hpp>

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
