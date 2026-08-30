#include <axiom/assets/LoaderRegistry.h>
#include <axiom/assets/VFS.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <unordered_map>

using json = nlohmann::json;

namespace axiom {

    // === ModManifest ===

    ModManifest ModManifest::LoadFromJson(const std::string &jsonText) {
        try {
            json j = json::parse(jsonText);
            ModManifest manifest;
            if (j.contains("mod_name"))
                manifest.mod_name = j["mod_name"].get<std::string>();
            if (j.contains("version"))
                manifest.version = j["version"].get<std::string>();
            if (j.contains("author"))
                manifest.author = j["author"].get<std::string>();
            if (j.contains("description"))
                manifest.description = j["description"].get<std::string>();

            if (j.contains("overrides")) {
                for (const auto &override : j["overrides"]) {
                    ModOverrideEntry entry;
                    entry.target_uuid = override["target_uuid"].get<std::string>();
                    entry.replacement_path = override["replacement_path"].get<std::string>();
                    manifest.overrides.push_back(entry);
                }
            }

            return manifest;
        } catch (const std::exception &e) {
            std::cerr << "Fehler beim Parsen von Mod-Manifest: " << e.what() << "\n";
            return ModManifest();
        }
    }

    std::string ModManifest::SaveToJson(bool pretty) const {
        json j;
        j["mod_name"] = mod_name;
        j["version"] = version;
        j["author"] = author;
        j["description"] = description;

        json overridesArray = json::array();
        for (const auto &override : overrides) {
            json overrideObj;
            overrideObj["target_uuid"] = override.target_uuid;
            overrideObj["replacement_path"] = override.replacement_path;
            overridesArray.push_back(overrideObj);
        }
        j["overrides"] = overridesArray;

        return pretty ? j.dump(2) : j.dump();
    }

    // === ModSystem ===

    static inline std::unordered_map<std::string, ModManifest> g_LoadedModManifests;
    static inline std::vector<std::string> g_LoadedMods;

    void ModSystem::RegisterMod(const std::string &modName, const std::string &modPath) {
        // Mount den Mod
        VFS::Mount(modName, modPath, VFS::MountType::Directory, true, 50);

        // Lade das Manifest
        std::string manifestPath = modName + "://manifest.json";
        std::string manifestText;
        if (VFS::ReadTextFile(manifestPath, manifestText)) {
            auto manifest = ModManifest::LoadFromJson(manifestText);
            g_LoadedModManifests[modName] = manifest;
            g_LoadedMods.push_back(modName);

            std::cout << "Mod registriert: " << modName << "\n";
        } else {
            std::cerr << "Mod-Manifest nicht gefunden: " << manifestPath << "\n";
        }
    }

    ModManifest ModSystem::GetModManifest(const std::string &modName) {
        auto it = g_LoadedModManifests.find(modName);
        if (it != g_LoadedModManifests.end()) {
            return it->second;
        }
        return ModManifest();
    }

    void ModSystem::ApplyModOverrides(const std::string &modName) {
        auto manifest = GetModManifest(modName);
        if (manifest.mod_name.empty()) {
            return;
        }

        auto registry = AssetManager::GetRegistry();
        for (const auto &override : manifest.overrides) {
            TypedUUID targetUUID = TypedUUID::FromString(override.target_uuid);
            registry->RegisterOverride(targetUUID, override.replacement_path);

            std::cout << "Override angewendet: " << override.target_uuid << " → "
                      << override.replacement_path << "\n";
        }
    }

    void ModSystem::RemoveModOverrides(const std::string &modName) {
        // TODO: Implementiere Override-Entfernung
        // Das erfordert ein Tracking der Überrides pro Mod
    }

    std::vector<std::string> ModSystem::GetLoadedMods() { return g_LoadedMods; }

} // namespace axiom
