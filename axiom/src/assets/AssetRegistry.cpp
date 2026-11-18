/**
 * @file axiom/src/assets/AssetRegistry.cpp
 * @brief Implements the asset registry module.
 */

#include <axiom/assets/AssetRegistry.h>
#include <algorithm>
#include <iostream>

namespace axiom {

    void AssetRegistry::init() {
        // Initialisiere Mount-Prioritäten
        sMountPriorities["engine"] = 0;  // Niedrigste
        sMountPriorities["game"] = 10;
        // Mod-Mounts erhalten höhere Prioritäten beim Registrieren
    }

    void AssetRegistry::shutdown() {
        sEntries.clear();
        sOverrides.clear();
        sMountPriorities.clear();
    }

    const AssetRegistryEntry *AssetRegistry::get(TypedUUID uuid) {
        auto it = sEntries.find(uuid);
        if (it != sEntries.end()) {
            return &it->second;
        }
        return nullptr;
    }

    std::string AssetRegistry::resolvePhysicalPath(TypedUUID uuid) {
        // Prüfe Overrides (höchste Priorität)
        auto overrideIt = sOverrides.find(uuid);
        if (overrideIt != sOverrides.end()) {
            return overrideIt->second.replacementPath;
        }

        // Fallback auf Registry-Eintrag
        auto entry = get(uuid);
        if (entry) {
            return entry->physicalPath;
        }

        return "";
    }

    void AssetRegistry::registerAsset(const AssetRegistryEntry &entry) {
        auto it = sEntries.find(entry.uuid);
        if (it != sEntries.end()) {
            // Eintrag existiert bereits - nur überschreiben falls diese Mount höhere
            // Priorität hat
            auto oldPriority = sMountPriorities.count(it->second.mountName)
                                   ? sMountPriorities[it->second.mountName]
                                   : 0;
            auto newPriority = sMountPriorities.count(entry.mountName)
                                   ? sMountPriorities[entry.mountName]
                                   : 0;

            if (newPriority >= oldPriority) {
                sEntries[entry.uuid] = entry;
            }
        } else {
            sEntries[entry.uuid] = entry;
        }
    }

    void AssetRegistry::unregister(TypedUUID uuid) {
        sEntries.erase(uuid);
        sOverrides.erase(uuid);
    }

    const std::vector<AssetDependency> &AssetRegistry::getDependencies(TypedUUID uuid) {
        auto it = sEntries.find(uuid);
        if (it != sEntries.end()) {
            return it->second.dependencies;
        }

        static const std::vector<AssetDependency> empty;
        return empty;
    }

    bool AssetRegistry::isHardDependency(TypedUUID from, TypedUUID to) {
        const auto &deps = getDependencies(from);
        for (const auto &dep : deps) {
            if (dep.targetUUID == to && dep.depType == AssetDependencyType::hard) {
                return true;
            }
        }
        return false;
    }

    void AssetRegistry::registerOverride(TypedUUID target,
                                          const std::string &replacementPath) {
        sOverrides[target] = {replacementPath, 100};
    }

    void AssetRegistry::clearOverrides() { sOverrides.clear(); }

    void AssetRegistry::scanMount(const std::string &mountName, bool isModMount) {
        if (isModMount) {
            // Mod-Mounts erhalten höhere Priorität basierend auf Lade-Reihenfolge
            static int modPriority = 50;
            sMountPriorities[mountName] = modPriority++;
        }
        // TODO: Implementiere Mount-Scanning hier
        // - VFS::ListFiles() für Mount aufrufen
        // - Metadaten-Dateien (.axmeta) parsen
        // - Abhängigkeiten extrahieren
        // - registerAsset() aufrufen
    }

    void AssetRegistry::rebuildFromMounts() {
        sEntries.clear();

        // Scan in Prioritäts-Reihenfolge: Engine → Game → Mods
        for (const auto &[mountName, priority] : sMountPriorities) {
            scanMount(mountName, priority >= 50);
        }

        if (!validateDependencyGraph()) {
            std::cerr << "AssetRegistry: Zyklen in Abhängigkeitsgraph erkannt!\n";
        }
    }

    bool AssetRegistry::validateDependencyGraph() {
        std::unordered_set<TypedUUID> visited;
        std::unordered_set<TypedUUID> inStack;

        for (const auto &[uuid, entry] : sEntries) {
            if (visited.find(uuid) == visited.end()) {
                if (!detectCycles(uuid, visited, inStack)) {
                    return false;
                }
            }
        }

        return true;
    }

    bool AssetRegistry::detectCycles(TypedUUID start, std::unordered_set<TypedUUID> &visited,
                                      std::unordered_set<TypedUUID> &inStack) {
        visited.insert(start);
        inStack.insert(start);

        const auto &deps = getDependencies(start);
        for (const auto &dep : deps) {
            if (dep.depType != AssetDependencyType::hard) {
                continue; // Nur hart-abhängige Zyklen prüfen
            }

            if (visited.find(dep.targetUUID) == visited.end()) {
                if (!detectCycles(dep.targetUUID, visited, inStack)) {
                    return false;
                }
            } else if (inStack.find(dep.targetUUID) != inStack.end()) {
                std::cerr << "Zyklus erkannt: "
                          << start.toString() << " → " << dep.targetUUID.toString() << "\n";
                return false;
            }
        }

        inStack.erase(start);
        return true;
    }

    size_t AssetRegistry::getAssetCount() { return sEntries.size(); }

    size_t AssetRegistry::getTotalAssetSize() {
        size_t total = 0;
        for (const auto &[uuid, entry] : sEntries) {
            total += entry.sizeBytes;
        }
        return total;
    }

} // namespace axiom
