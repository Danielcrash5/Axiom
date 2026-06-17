#pragma once

#include "axiom/core/Application.h"
#include "axiom/core/Layer.h"
#include "axiom/core/Logger.h"
#include "axiom/core/Time.h"
#include "axiom/core/UUID.h"

#include "axiom/events/EventBus.h"
#include "axiom/events/Events.h"

#include "axiom/input/KeyCodes.h"
#include "axiom/input/MouseCodes.h"
#include "axiom/input/GamepadCodes.h"
#include "axiom/input/Input.h"
#include "axiom/input/InputSystem.h"
#include "axiom/input/ActionMap.h"

#include "axiom/assets/VFS.h"
#include "axiom/assets/AssetManager.h"

//#include "axiom/renderer/Renderer.h"

#include "axiom/ecs/Components.h"
#include "axiom/ecs/Entity.h"
#include "axiom/ecs/Scene.h"
#include "axiom/ecs/SystemManager.h"
#include "axiom/ecs/Render2DSystem.h"
#include "axiom/ecs/ISystem.h"

#include "axiom/profiling/Profiler.h"
#include "axiom/ImGui/IImGuiPanel.h"

namespace axiom {

Application* CreateApplication();

}
