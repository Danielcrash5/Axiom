#include <axiom/assets/AssetRegistry.h>
#include <algorithm>
#include <iostream>

namespace axiom {

    void AssetRegistry::Init() {
        // Initialisiere Mount-Prioritäten
        s_MountPriorities["engine"] = 0;  // Niedrigste
        s_MountPriorities["game"] = 10;
        // Mod-Mounts erhalten höhere Prioritäten beim Registrieren
    }

    void AssetRegistry::Shutdown() {
        s_Entries.clear();
        s_Overrides.clear();
        s_MountPriorities.clear();
    }

    const AssetRegistryEntry *AssetRegistry::Get(TypedUUID uuid) {
        auto it = s_Entries.find(uuid);
        if (it != s_Entries.end()) {
            return &it->second;
        }
        return nullptr;
    }

    std::string AssetRegistry::ResolvePhysicalPath(TypedUUID uuid) {
        // Prüfe Overrides (höchste Priorität)
        auto overrideIt = s_Overrides.find(uuid);
        if (overrideIt != s_Overrides.end()) {
            return overrideIt->second.replacementPath;
        }

        // Fallback auf Registry-Eintrag
        auto entry = Get(uuid);
        if (entry) {
            return entry->physicalPath;
        }

        return "";
    }

    void AssetRegistry::Register(const AssetRegistryEntry &entry) {
        auto it = s_Entries.find(entry.uuid);
        if (it != s_Entries.end()) {
            // Eintrag existiert bereits - nur überschreiben falls diese Mount höhere
            // Priorität hat
            auto oldPriority = s_MountPriorities.count(it->second.mountName)
                                   ? s_MountPriorities[it->second.mountName]
                                   : 0;
            auto newPriority = s_MountPriorities.count(entry.mountName)
                                   ? s_MountPriorities[entry.mountName]
                                   : 0;

            if (newPriority >= oldPriority) {
                s_Entries[entry.uuid] = entry;
            }
        } else {
            s_Entries[entry.uuid] = entry;
        }
    }

    void AssetRegistry::Unregister(TypedUUID uuid) {
        s_Entries.erase(uuid);
        s_Overrides.erase(uuid);
    }

    const std::vector<AssetDependency> &AssetRegistry::GetDependencies(TypedUUID uuid) {
        auto it = s_Entries.find(uuid);
        if (it != s_Entries.end()) {
            return it->second.dependencies;
        }

        static const std::vector<AssetDependency> empty;
        return empty;
    }

    bool AssetRegistry::IsHardDependency(TypedUUID from, TypedUUID to) {
        const auto &deps = GetDependencies(from);
        for (const auto &dep : deps) {
            if (dep.targetUUID == to && dep.depType == AssetDependencyType::Hard) {
                return true;
            }
        }
        return false;
    }

    void AssetRegistry::RegisterOverride(TypedUUID target,
                                          const std::string &replacementPath) {
        s_Overrides[target] = {replacementPath, 100};
    }

    void AssetRegistry::ClearOverrides() { s_Overrides.clear(); }

    void AssetRegistry::ScanMount(const std::string &mountName, bool isModMount) {
        if (isModMount) {
            // Mod-Mounts erhalten höhere Priorität basierend auf Lade-Reihenfolge
            static int modPriority = 50;
            s_MountPriorities[mountName] = modPriority++;
        }
        // TODO: Implementiere Mount-Scanning hier
        // - VFS::ListFiles() für Mount aufrufen
        // - Metadaten-Dateien (.axmeta) parsen
        // - Abhängigkeiten extrahieren
        // - Register() aufrufen
    }

    void AssetRegistry::RebuildFromMounts() {
        s_Entries.clear();

        // Scan in Prioritäts-Reihenfolge: Engine → Game → Mods
        for (const auto &[mountName, priority] : s_MountPriorities) {
            ScanMount(mountName, priority >= 50);
        }

        if (!ValidateDependencyGraph()) {
            std::cerr << "AssetRegistry: Zyklen in Abhängigkeitsgraph erkannt!\n";
        }
    }

    bool AssetRegistry::ValidateDependencyGraph() {
        std::unordered_set<TypedUUID> visited;
        std::unordered_set<TypedUUID> inStack;

        for (const auto &[uuid, entry] : s_Entries) {
            if (visited.find(uuid) == visited.end()) {
                if (!DetectCycles(uuid, visited, inStack)) {
                    return false;
                }
            }
        }

        return true;
    }

    bool AssetRegistry::DetectCycles(TypedUUID start, std::unordered_set<TypedUUID> &visited,
                                      std::unordered_set<TypedUUID> &inStack) {
        visited.insert(start);
        inStack.insert(start);

        const auto &deps = GetDependencies(start);
        for (const auto &dep : deps) {
            if (dep.depType != AssetDependencyType::Hard) {
                continue; // Nur hart-abhängige Zyklen prüfen
            }

            if (visited.find(dep.targetUUID) == visited.end()) {
                if (!DetectCycles(dep.targetUUID, visited, inStack)) {
                    return false;
                }
            } else if (inStack.find(dep.targetUUID) != inStack.end()) {
                std::cerr << "Zyklus erkannt: "
                          << start.ToString() << " → " << dep.targetUUID.ToString() << "\n";
                return false;
            }
        }

        inStack.erase(start);
        return true;
    }

    size_t AssetRegistry::GetAssetCount() { return s_Entries.size(); }

    size_t AssetRegistry::GetTotalAssetSize() {
        size_t total = 0;
        for (const auto &[uuid, entry] : s_Entries) {
            total += entry.sizeBytes;
        }
        return total;
    }

} // namespace axiom
