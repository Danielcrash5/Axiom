# TODO

## Prefab-System

Die Prefab-Arbeiten aus den Asset-System-Anforderungen sind bewusst zurückgestellt:

- `SceneEntity::prefabInstance` auf `std::optional<PrefabInstanceData>` umstellen.
- `SceneEntity::ToJson` und `FromJson` an die optionale Prefab-Instanz anpassen.
- Einen AddressSanitizer-Test für wiederholte Kopien und Reallokationen von `std::vector<SceneEntity>` ergänzen.
- `SceneManager::InstantiatePrefab` mit `entt::registry&` implementieren.
- `SceneManager::ReloadPrefab` implementieren: Prefab-Instanzen finden, Daten neu laden und nur nicht überschriebenen Komponenten aktualisieren.
- Einen Test für zwei Prefab-Instanzen mit Override und Reload ergänzen.

## Nächste Schritte

1. Erster sichtbarer Quad (Phase 4): Vulkan-GLSL-Shader, Einheits-Quad-Mesh, Entity-`RenderPass`, System `TransformComponent` → `RenderItem`, `CameraComponent` → View-Matrizen.
2. Phase 9 vor Phase 7: Luau-Kern-Wrapper, danach Reflection-Integration. Actor/Future/Scheduler erst später.
3. Reflection-Code aus dem Testprojekt (C++26, GCC 16.1) ins Haupt-Repo übernehmen.

## Offene Punkte / Aufräumen

- `Scene::GetEntityByUUID` fehlt: `IDComponent::ID` (128 Bit) wird erzeugt, aber nirgends als Lookup-Schlüssel oder in der Serialisierung genutzt. `UUIDSerialization.h` schreibt jetzt die vollen 128 Bit, wird aber noch nirgends inkludiert.
- Inspector: Quaternion↔Euler wird aktuell jeden Frame umgerechnet (Platzhalter in `InspectorPanel.cpp`). Das führt zu Winkelsprüngen. Fix: zuletzt editierte Euler-Winkel pro Entity (Schlüssel `IDComponent::ID`) im Panel-State cachen.
- `Asset.h` / `AssetMetaData.h` haben keine Konsumenten, bleiben aber bewusst vorerst drin.
- ImGuizmo: `add_subdirectory` und Linking in der CMake fehlen (Compile-/Linkfehler).
- Multi-Viewport: Verhalten bei minimiertem Hauptfenster ist ungetestet (vermutlich werden die Sekundärfenster weiter gerendert, obwohl der Renderer den Hauptfenster-Frame überspringt).
- Bekannte, harmlose Validation-Meldung `UNASSIGNED-non-acquired-swapchain-image-used` beim schnellen Resize ausgedockter ImGui-Fenster (vermutlich ein Race im ImGui-Vulkan-Backend beim Neuaufbau der Sekundär-Swapchain; nicht abschließend geklärt, auf TRACE gesenkt). Die Ursache ist nicht behoben.
- shaderc (glslang + SPIRV-Tools) ist über `axiom_engine` auch im Runtime-Build gelinkt, obwohl dort nie ein Shader kompiliert wird. Spätere Trennung in Core-/Editor-Library prüfen.
- C++-Hot-Reload (DLL entladen/neu laden): Callback-Tagging muss pro Instanz (HMODULE bzw. Luau-State) erfolgen, nicht pro Kategorie. Tagging allein deckt keine Engine-Callbacks ab, die Rohpointer auf DLL-Objekte capturen; dort Handle/ID statt Rohpointer.
- README: Phase 1 (OpenGL-Backend) und Phase 3 (EnTT) beschreiben nicht mehr den aktuellen Stand (Vulkan-RHI, flecs).
