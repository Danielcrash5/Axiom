#pragma once
#include <axiom/assets/TypedUUID.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace axiom {

    enum class AssetDependencyType { Hard = 0, Soft = 1 };

    // Beschreibt eine Abhängigkeit zwischen zwei Assets
    struct AssetDependency {
        TypedUUID targetUUID;         // Das abhängige Asset
        AssetDependencyType depType;  // Hart (wird mit geladen) oder weich (Laufzeit)
        int priority = 0;             // Für Lade-Reihenfolge bei mehreren
    };

    // Registry-Eintrag für ein Asset
    struct AssetRegistryEntry {
        TypedUUID uuid;
        std::string physicalPath;    // Mount://pfad
        std::string mountName;       // z.B. "game", "engine", "ModName"
        std::vector<AssetDependency> dependencies;
        size_t sizeBytes = 0;        // Metadata
        uint64_t lastModified = 0;   // Für Editor Hot-Reload
    };

    // Zentrale Registry, bildet TypedUUID → physische Orte (mit Mod-Overrides)
    class AssetRegistry {
      public:
        static void Init();
        static void Shutdown();

        // Basis-API
        [[nodiscard]] static const AssetRegistryEntry *Get(TypedUUID uuid);
        [[nodiscard]] static std::string ResolvePhysicalPath(TypedUUID uuid);

        // Registrierung (vom Scanner aufgerufen)
        static void Register(const AssetRegistryEntry &entry);
        static void Unregister(TypedUUID uuid);

        // Abhängigkeitsgraph
        [[nodiscard]] static const std::vector<AssetDependency> &
        GetDependencies(TypedUUID uuid);
        [[nodiscard]] static bool IsHardDependency(TypedUUID from, TypedUUID to);

        // Mod-Overrides
        static void RegisterOverride(TypedUUID target, const std::string &replacementPath);
        static void ClearOverrides();

        // Scanning/Rebuild (mit Priorität: Mod > Game > Engine)
        static void ScanMount(const std::string &mountName, bool isModMount = false);
        static void RebuildFromMounts();

        // Integrität prüfen (erkennt Zyklen in harthen Abhängigkeiten)
        [[nodiscard]] static bool ValidateDependencyGraph();

        // Statistics
        [[nodiscard]] static size_t GetAssetCount();
        [[nodiscard]] static size_t GetTotalAssetSize();

      private:
        struct Override {
            std::string replacementPath;
            int priority; // Höher = höhere Priorität
        };

        static inline std::unordered_map<TypedUUID, AssetRegistryEntry> s_Entries;
        static inline std::unordered_map<TypedUUID, Override> s_Overrides;
        static inline std::unordered_map<std::string, int> s_MountPriorities;

        [[nodiscard]] static bool DetectCycles(TypedUUID start,
                                               std::unordered_set<TypedUUID> &visited,
                                               std::unordered_set<TypedUUID> &inStack);
    };

} // namespace axiom