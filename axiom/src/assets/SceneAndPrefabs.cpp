/**
 * @file axiom/src/assets/SceneAndPrefabs.cpp
 * @brief Implements the scene and prefabs module.
 */

#include <axiom/assets/SceneAndPrefabs.h>
#include <axiom/core/Logger.h>
#include <iostream>
#include <nlohmann/json.hpp>

namespace axiom {
    using json = nlohmann::json;

    // === SceneEntity ===

    std::string SceneEntity::toJson() const {
        json j;
        j["id"] = id;
        j["name"] = name;
        j["active"] = active;
        // Parse components string if it's valid JSON, otherwise store as-is
        try {
            j["components"] = json::parse(components);
        } catch (...) {
            j["components"] = components;
        }

        if (prefabInstance) {
            j["isPrefabInstance"] = true;
            j["prefabUUID"] = prefabInstance->prefabUUID.toString();
            json overridesArray = json::array();
            for (const auto &override : prefabInstance->overrides) {
                json overrideObj;
                overrideObj["component"] = override.componentName;
                try {
                    overrideObj["properties"] =
                        json::parse(override.propertyOverrides);
                } catch (...) {
                    overrideObj["properties"] = override.propertyOverrides;
                }
                overridesArray.push_back(overrideObj);
            }
            j["overrides"] = overridesArray;
        }

        return j.dump();
    }

    SceneEntity SceneEntity::fromJson(const std::string &jsonStr) {
        SceneEntity entity;
        try {
            json j = json::parse(jsonStr);
            if (j.contains("id"))
                entity.id = j["id"].get<uint32_t>();
            if (j.contains("name"))
                entity.name = j["name"].get<std::string>();
            if (j.contains("active"))
                entity.active = j["active"].get<bool>();
            if (j.contains("components"))
                entity.components = j["components"].dump();

            if (j.value("isPrefabInstance", false)) {
                entity.prefabInstance =
                    PrefabInstanceData{}; // statt: new PrefabInstanceData()
                entity.prefabInstance->prefabUUID =
                    TypedUUID::fromString(j["prefabUUID"].get<std::string>());

                if (j.contains("overrides")) {
                    for (const auto &override : j["overrides"]) {
                        PrefabInstanceOverride ovr;
                        ovr.componentName =
                            override["component"].get<std::string>();
                        ovr.propertyOverrides = override["properties"].dump();
                        entity.prefabInstance->overrides.push_back(ovr);
                    }
                }
            }
        } catch (const std::exception &e) {
            std::cerr << "Error parsing SceneEntity JSON: " << e.what() << "\n";
        }
        return entity;
    }

    // === SceneData ===

    std::string SceneData::toJson() const {
        json j;
        j["name"] = name;
        j["nextEntityId"] = nextEntityId;
        json entitiesArray = json::array();
        for (const auto &entity : entities) {
            entitiesArray.push_back(json::parse(entity.toJson()));
        }
        j["entities"] = entitiesArray;
        return j.dump();
    }

    SceneData SceneData::fromJson(const std::string &jsonStr) {
        SceneData scene;
        try {
            json j = json::parse(jsonStr);
            if (j.contains("name"))
                scene.name = j["name"].get<std::string>();
            if (j.contains("nextEntityId"))
                scene.nextEntityId = j["nextEntityId"].get<uint32_t>();

            if (j.contains("entities")) {
                for (const auto &entityJson : j["entities"]) {
                    scene.entities.push_back(
                        SceneEntity::fromJson(entityJson.dump()));
                }
            }
        } catch (const std::exception &e) {
            std::cerr << "Error parsing SceneData JSON: " << e.what() << "\n";
        }
        return scene;
    }

    SceneData SceneData::loadFromJson(const std::string &jsonText) {
        return fromJson(jsonText);
    }

    std::string SceneData::saveToJson(bool pretty) const {
        try {
            json j = json::parse(toJson());
            return pretty ? j.dump(2) : j.dump();
        } catch (...) {
            return toJson();
        }
    }

    // === PrefabData ===

    std::string PrefabData::toJson() const {
        json j;
        j["name"] = name;
        j["baseEntityUUID"] = baseEntityUUID.toString();
        j["templateEntity"] = json::parse(templateEntity.toJson());

        json depsArray = json::array();
        for (const auto &dep : componentDependencies) {
            json depObj;
            depObj["target"] = dep.targetUUID.toString();
            depObj["type"] = static_cast<int>(dep.depType);
            depsArray.push_back(depObj);
        }
        j["componentDependencies"] = depsArray;

        return j.dump();
    }

    PrefabData PrefabData::fromJson(const std::string &jsonStr) {
        PrefabData prefab;
        try {
            json j = json::parse(jsonStr);
            if (j.contains("name"))
                prefab.name = j["name"].get<std::string>();
            if (j.contains("baseEntityUUID"))
                prefab.baseEntityUUID = TypedUUID::fromString(
                    j["baseEntityUUID"].get<std::string>());
            if (j.contains("templateEntity"))
                prefab.templateEntity =
                    SceneEntity::fromJson(j["templateEntity"].dump());

            if (j.contains("componentDependencies")) {
                for (const auto &dep : j["componentDependencies"]) {
                    AssetDependency dependency;
                    dependency.targetUUID =
                        TypedUUID::fromString(dep["target"].get<std::string>());
                    dependency.depType = static_cast<AssetDependencyType>(
                        dep["type"].get<int>());
                    prefab.componentDependencies.push_back(dependency);
                }
            }
        } catch (const std::exception &e) {
            std::cerr << "Error parsing PrefabData JSON: " << e.what() << "\n";
        }
        return prefab;
    }

    PrefabData PrefabData::loadFromJson(const std::string &jsonText) {
        return fromJson(jsonText);
    }

    std::string PrefabData::saveToJson(bool pretty) const {
        try {
            json j = json::parse(toJson());
            return pretty ? j.dump(2) : j.dump();
        } catch (...) {
            return toJson();
        }
    }

    // === SceneManager ===

    AssetHandle<SceneAsset> SceneManager::loadScene(TypedUUID sceneId) {
        AXIOM_WARN("SceneManager::LoadScene is not implemented for scene {}",
                   sceneId.toString());
        return AssetHandle<SceneAsset>();
    }

    uint32_t SceneManager::instantiatePrefab(AssetHandle<PrefabAsset> prefab,
                                             uint32_t parentRegistryVersion) {
        AXIOM_ASSERT(prefab.isValid(), "SceneManager::InstantiatePrefab "
                                       "received an invalid prefab handle");
        AXIOM_ERROR("SceneManager::InstantiatePrefab reached an unavailable "
                    "prefab implementation "
                    "for {} (parent registry version {})",
                    prefab.getId().toString(), parentRegistryVersion);
        AXIOM_ASSERT(false,
                     "SceneManager::InstantiatePrefab is not implemented");
        return 0;
    }

    void SceneManager::reloadPrefab(TypedUUID prefabId) {
        AXIOM_ASSERT(
            prefabId.isValid(),
            "SceneManager::ReloadPrefab received an invalid prefab UUID");
        AXIOM_ERROR("SceneManager::ReloadPrefab reached an unavailable prefab "
                    "implementation for {}",
                    prefabId.toString());
        AXIOM_ASSERT(false, "SceneManager::ReloadPrefab is not implemented");
    }

} // namespace axiom
