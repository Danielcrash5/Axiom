/**
 * @file axiom/include/axiom/assets/SceneAndPrefabs.h
 * @brief Declares the public interface for the scene and prefabs.
 */

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

    /** @brief Stores the data for the prefab instance override. */
    struct PrefabInstanceOverride {
        /** @brief Stores the component name. */
        std::string componentName;
        /** @brief Stores the property overrides. */
        std::string propertyOverrides; // Serialized JSON string
    };

    /** @brief Stores the data for the prefab instance data. */
    struct PrefabInstanceData {
        /** @brief Stores the prefab UUID. */
        TypedUUID prefabUUID;
        /** @brief Stores the overrides. */
        std::vector<PrefabInstanceOverride> overrides;
    };

    // === Scene Format (JSON, editierbar) ===

    /** @brief Stores the data for the scene entity. */
    struct SceneEntity {
        /** @brief Stores the ID. */
        uint32_t id = 0;
        /** @brief Stores the name. */
        std::string name;
        /** @brief Stores the active. */
        bool active = true;
        /** @brief Stores the components. */
        std::string components; // Serialized JSON string

        // Falls diese Entity eine Prefab-Instanz ist:
        /** @brief Stores the prefab instance. */
        std::optional<PrefabInstanceData> prefabInstance;

        /**
         * @brief Converts this value to JSON.
         * @return The result of the operation.
         */
        std::string toJson() const;
        /**
         * @brief Creates a value from JSON data.
         * @param jsonStr The JSON str used by this operation.
         * @return The result of the operation.
         */
        static SceneEntity fromJson(const std::string &jsonStr);
    };

    /** @brief Stores the data for the scene data. */
    struct SceneData {
        /** @brief Stores the name. */
        std::string name;
        /** @brief Stores the next entity ID. */
        uint32_t nextEntityId = 1;
        /** @brief Stores the entities. */
        std::vector<SceneEntity> entities;

        /**
         * @brief Converts this value to JSON.
         * @return The result of the operation.
         */
        std::string toJson() const;
        /**
         * @brief Creates a value from JSON data.
         * @param jsonStr The JSON str used by this operation.
         * @return The result of the operation.
         */
        static SceneData fromJson(const std::string &jsonStr);
        /**
         * @brief Loads from JSON.
         * @param jsonText The JSON text used by this operation.
         * @return The result of the operation.
         */
        static SceneData loadFromJson(const std::string &jsonText);
        /**
         * @brief Saves to JSON.
         * @param pretty The pretty used by this operation.
         * @return The result of the operation.
         */
        std::string saveToJson(bool pretty = true) const;
    };

    // === Prefab Format (JSON, editierbar) ===

    /** @brief Stores the data for the prefab data. */
    struct PrefabData {
        /** @brief Stores the name. */
        std::string name;
        /** @brief Stores the base entity UUID. */
        TypedUUID baseEntityUUID; // Eindeutige ID dieser Prefab
        /** @brief Stores the template entity. */
        SceneEntity templateEntity;
        /** @brief Stores the component dependencies. */
        std::vector<AssetDependency> componentDependencies;

        /**
         * @brief Converts this value to JSON.
         * @return The result of the operation.
         */
        std::string toJson() const;
        /**
         * @brief Creates a value from JSON data.
         * @param jsonStr The JSON str used by this operation.
         * @return The result of the operation.
         */
        static PrefabData fromJson(const std::string &jsonStr);
        /**
         * @brief Loads from JSON.
         * @param jsonText The JSON text used by this operation.
         * @return The result of the operation.
         */
        static PrefabData loadFromJson(const std::string &jsonText);
        /**
         * @brief Saves to JSON.
         * @param pretty The pretty used by this operation.
         * @return The result of the operation.
         */
        std::string saveToJson(bool pretty = true) const;
    };

    // === Runtime Scene Management ===

    /**
     * @brief Provides the scene manager functionality.
     * @tparam SceneManager Template type used by this type.
     */
    class SceneManager {
      public:
        // Lade eine Szene asynchron
        /**
         * @brief Loads scene.
         * @param sceneId The scene ID used by this operation.
         * @return The result of the operation.
         */
        static AssetHandle<SceneAsset> loadScene(TypedUUID sceneId);

        // Instanziiere eine Entity aus einer Prefab in einer laufenden Registry
        // Gibt die neue Entity-ID zurück
        /**
         * @brief Creates entities from the prefab data.
         * @param prefab The prefab used by this operation.
         * @param parentRegistryVersion The parent registry version used by this operation.
         * @return The result of the operation.
         */
        static uint32_t instantiatePrefab(AssetHandle<PrefabAsset> prefab,
                                          uint32_t parentRegistryVersion = 0);

        // Hot-Reload: Laden Sie eine Prefab neu und wenden Sie Änderungen auf alle Instanzen an
        // Respektiert Overrides
        /**
         * @brief Reloads prefab.
         * @param prefabId The prefab ID used by this operation.
         */
        static void reloadPrefab(TypedUUID prefabId);
    };

} // namespace axiom
