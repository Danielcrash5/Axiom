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
#include "axiom/assets/TextureLoadInfo.h"

#include "axiom/renderer/Renderer.h"
#include "axiom/renderer/Renderer2D.h"
#include "axiom/renderer/RenderCommand.h"
#include "axiom/renderer/RenderState.h"
#include "axiom/renderer/Shader.h"
#include "axiom/renderer/Texture2D.h"
#include "axiom/renderer/Material.h"
#include "axiom/renderer/Sprite.h"
#include "axiom/renderer/Model.h"
#include "axiom/renderer/Mesh.h"
#include "axiom/renderer/VertexBuffer.h"
#include "axiom/renderer/VertexArray.h"
#include "axiom/renderer/IndexBuffer.h"
#include "axiom/renderer/BufferLayout.h"
#include "axiom/renderer/IndirectDrawBuffer.h"
#include "axiom/renderer/SubMesh.h"
#include "axiom/renderer/TextureOptions.h"
#include "axiom/renderer/ShaderStage.h"
#include "axiom/renderer/ShaderSource.h"
#include "axiom/renderer/ShaderPreprocessor.h"
#include "axiom/renderer/ShaderDataType.h"

#include "axiom/ecs/Components.h"
#include "axiom/ecs/Entity.h"
#include "axiom/ecs/Scene.h"
#include "axiom/ecs/SystemManager.h"
#include "axiom/ecs/Render2DSystem.h"
#include "axiom/ecs/ISystem.h"

#include "axiom/profiling/Profiler.h"

//TEMP: ImGui kurz testen besseres System später 
#include <imgui.h>

namespace axiom {

Application* CreateApplication();

}
