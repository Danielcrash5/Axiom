#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <axiom/assets/TypedUUID.h>
#include <axiom/assets/AssetRegistry.h>
#include <axiom/assets/AssetHandle.h>
#include <axiom/assets/AssetTypes.h>

namespace axiom {

    // === Prefab Instancing ===

    struct PrefabInstanceOverride {
        std::string componentName;
        std::string propertyOverrides; // Serialized JSON string
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
        std::string components; // Serialized JSON string

        // Falls diese Entity eine Prefab-Instanz ist:
        std::optional<PrefabInstanceData> prefabInstance;

        std::string ToJson() const;
        static SceneEntity FromJson(const std::string &jsonStr);
    };

    struct SceneData {
        std::string name;
        uint32_t nextEntityId = 1;
        std::vector<SceneEntity> entities;

        std::string ToJson() const;
        static SceneData FromJson(const std::string &jsonStr);
        static SceneData LoadFromJson(const std::string &jsonText);
        std::string SaveToJson(bool pretty = true) const;
    };

    // === Prefab Format (JSON, editierbar) ===

    struct PrefabData {
        std::string name;
        TypedUUID baseEntityUUID; // Eindeutige ID dieser Prefab
        SceneEntity templateEntity;
        std::vector<AssetDependency> componentDependencies;

        std::string ToJson() const;
        static PrefabData FromJson(const std::string &jsonStr);
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
