#include <axiom/assets/SceneAndPrefabs.h>
#include <axiom/core/Logger.h>
#include <iostream>
#include <nlohmann/json.hpp>

namespace axiom {
    using json = nlohmann::json;

    // === SceneEntity ===

    std::string SceneEntity::ToJson() const {
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
            j["prefabUUID"] = prefabInstance->prefabUUID.ToString();
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

    SceneEntity SceneEntity::FromJson(const std::string &jsonStr) {
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
                    TypedUUID::FromString(j["prefabUUID"].get<std::string>());

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

    std::string SceneData::ToJson() const {
        json j;
        j["name"] = name;
        j["nextEntityId"] = nextEntityId;
        json entitiesArray = json::array();
        for (const auto &entity : entities) {
            entitiesArray.push_back(json::parse(entity.ToJson()));
        }
        j["entities"] = entitiesArray;
        return j.dump();
    }

    SceneData SceneData::FromJson(const std::string &jsonStr) {
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
                        SceneEntity::FromJson(entityJson.dump()));
                }
            }
        } catch (const std::exception &e) {
            std::cerr << "Error parsing SceneData JSON: " << e.what() << "\n";
        }
        return scene;
    }

    SceneData SceneData::LoadFromJson(const std::string &jsonText) {
        return FromJson(jsonText);
    }

    std::string SceneData::SaveToJson(bool pretty) const {
        try {
            json j = json::parse(ToJson());
            return pretty ? j.dump(2) : j.dump();
        } catch (...) {
            return ToJson();
        }
    }

    // === PrefabData ===

    std::string PrefabData::ToJson() const {
        json j;
        j["name"] = name;
        j["baseEntityUUID"] = baseEntityUUID.ToString();
        j["templateEntity"] = json::parse(templateEntity.ToJson());

        json depsArray = json::array();
        for (const auto &dep : componentDependencies) {
            json depObj;
            depObj["target"] = dep.targetUUID.ToString();
            depObj["type"] = static_cast<int>(dep.depType);
            depsArray.push_back(depObj);
        }
        j["componentDependencies"] = depsArray;

        return j.dump();
    }

    PrefabData PrefabData::FromJson(const std::string &jsonStr) {
        PrefabData prefab;
        try {
            json j = json::parse(jsonStr);
            if (j.contains("name"))
                prefab.name = j["name"].get<std::string>();
            if (j.contains("baseEntityUUID"))
                prefab.baseEntityUUID = TypedUUID::FromString(
                    j["baseEntityUUID"].get<std::string>());
            if (j.contains("templateEntity"))
                prefab.templateEntity =
                    SceneEntity::FromJson(j["templateEntity"].dump());

            if (j.contains("componentDependencies")) {
                for (const auto &dep : j["componentDependencies"]) {
                    AssetDependency dependency;
                    dependency.targetUUID =
                        TypedUUID::FromString(dep["target"].get<std::string>());
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

    PrefabData PrefabData::LoadFromJson(const std::string &jsonText) {
        return FromJson(jsonText);
    }

    std::string PrefabData::SaveToJson(bool pretty) const {
        try {
            json j = json::parse(ToJson());
            return pretty ? j.dump(2) : j.dump();
        } catch (...) {
            return ToJson();
        }
    }

    // === SceneManager ===

    AssetHandle<SceneAsset> SceneManager::LoadScene(TypedUUID sceneId) {
        AXIOM_WARN("SceneManager::LoadScene is not implemented for scene {}",
                   sceneId.ToString());
        return AssetHandle<SceneAsset>();
    }

    uint32_t SceneManager::InstantiatePrefab(AssetHandle<PrefabAsset> prefab,
                                             uint32_t parentRegistryVersion) {
        AXIOM_ASSERT(prefab.IsValid(), "SceneManager::InstantiatePrefab "
                                       "received an invalid prefab handle");
        AXIOM_ERROR("SceneManager::InstantiatePrefab reached an unavailable "
                    "prefab implementation "
                    "for {} (parent registry version {})",
                    prefab.GetID().ToString(), parentRegistryVersion);
        AXIOM_ASSERT(false,
                     "SceneManager::InstantiatePrefab is not implemented");
        return 0;
    }

    void SceneManager::ReloadPrefab(TypedUUID prefabId) {
        AXIOM_ASSERT(
            prefabId.IsValid(),
            "SceneManager::ReloadPrefab received an invalid prefab UUID");
        AXIOM_ERROR("SceneManager::ReloadPrefab reached an unavailable prefab "
                    "implementation for {}",
                    prefabId.ToString());
        AXIOM_ASSERT(false, "SceneManager::ReloadPrefab is not implemented");
    }

} // namespace axiom
