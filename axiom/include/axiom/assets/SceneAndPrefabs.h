#pragma once
#include <axiom/assets/TypedUUID.h>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

using json = nlohmann::json;

namespace axiom {

    // === Prefab Instancing ===

    struct PrefabInstanceOverride {
        std::string componentName;
        json propertyOverrides; // Property-Name → Wert
    };

    struct PrefabInstanceData {
        TypedUUID prefabUUID;
        std::vector<PrefabInstanceOverride> overrides;
    };

    // === Scene Format (JSON, editierbar) ===

    struct SceneEntity {
        uint32_t id = 0;
        std::string name;
        bool active = true;
        json components; // Component-Name → Komponenten-Daten

        // Falls diese Entity eine Prefab-Instanz ist:
        PrefabInstanceData *prefabInstance = nullptr;

        json ToJson() const;
        static SceneEntity FromJson(const json &j);
    };

    struct SceneData {
        std::string name;
        uint32_t nextEntityId = 1;
        std::vector<SceneEntity> entities;

        json ToJson() const;
        static SceneData FromJson(const json &j);
        static SceneData LoadFromJson(const std::string &jsonText);
        std::string SaveToJson(bool pretty = true) const;
    };

    // === Prefab Format (JSON, editierbar) ===

    struct PrefabData {
        std::string name;
        TypedUUID baseEntityUUID; // Eindeutige ID dieser Prefab
        SceneEntity templateEntity;
        std::vector<AssetDependency> componentDependencies;

        json ToJson() const;
        static PrefabData FromJson(const json &j);
        static PrefabData LoadFromJson(const std::string &jsonText);
        std::string SaveToJson(bool pretty = true) const;
    };

    // === Runtime Scene Management ===

    class SceneManager {
      public:
        // Lade eine Szene asynchron
        static AssetHandle<SceneAsset> LoadScene(TypedUUID sceneId);

        // Instanziiere eine Entity aus einer Prefab in einer laufenden Registry
        // Gibt die neue Entity-ID zurück
        static uint32_t InstantiatePrefab(AssetHandle<PrefabAsset> prefab,
                                          uint32_t parentRegistryVersion = 0);

        // Hot-Reload: Laden Sie eine Prefab neu und wenden Sie Änderungen auf alle Instanzen an
        // Respektiert Overrides
        static void ReloadPrefab(TypedUUID prefabId);
    };

} // namespace axiom
