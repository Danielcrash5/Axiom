/**
 * @file axiom/include/axiom/assets/AssetRegistry.h
 * @brief Declares the public interface for the asset registry.
 */

#pragma once
#include <axiom/assets/TypedUUID.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace axiom {

    /**
     * @brief Defines the available asset dependency type values.
     * @tparam AssetDependencyType Template type used by this type.
     */
    enum class AssetDependencyType { hard = 0, soft = 1 };

    // Beschreibt eine Abhängigkeit zwischen zwei Assets
    /** @brief Stores the data for the asset dependency. */
    struct AssetDependency {
        /** @brief Stores the target UUID. */
        TypedUUID targetUUID;         // Das abhängige Asset
        /** @brief Stores the dep type. */
        AssetDependencyType depType;  // Hart (wird mit geladen) oder weich (Laufzeit)
        /** @brief Stores the priority. */
        int priority = 0;             // Für Lade-Reihenfolge bei mehreren
    };

    // Registry-Eintrag für ein Asset
    /** @brief Stores the data for the asset registry entry. */
    struct AssetRegistryEntry {
        /** @brief Stores the UUID. */
        TypedUUID uuid;
        /** @brief Stores the physical path. */
        std::string physicalPath;    // Mount://pfad
        /** @brief Stores the mount name. */
        std::string mountName;       // z.B. "game", "engine", "ModName"
        /** @brief Stores the dependencies. */
        std::vector<AssetDependency> dependencies;
        /** @brief Stores the size in bytes. */
        size_t sizeBytes = 0;        // Metadata
        /** @brief Stores the last modified. */
        uint64_t lastModified = 0;   // Für Editor Hot-Reload
    };

    // Zentrale Registry, bildet TypedUUID → physische Orte (mit Mod-Overrides)
    /**
     * @brief Maps asset identifiers to physical locations and dependency metadata.
     * @tparam AssetRegistry Template type used by this type.
     */
    class AssetRegistry {
      public:
        /**
         * @brief Initializes the subsystem.
         */
        static void init();
        /**
         * @brief Shuts down the subsystem.
         */
        static void shutdown();

        // Basis-API
        /**
         * @brief Returns the requested value.
         * @param uuid The UUID used by this operation.
         * @return The result of the operation.
         */
        [[nodiscard]] static const AssetRegistryEntry *get(TypedUUID uuid);
        /**
         * @brief Resolves physical path.
         * @param uuid The UUID used by this operation.
         * @return The result of the operation.
         */
        [[nodiscard]] static std::string resolvePhysicalPath(TypedUUID uuid);

        // Registrierung (vom Scanner aufgerufen)
        /**
         * @brief Registers asset.
         * @param entry The entry used by this operation.
         */
        static void registerAsset(const AssetRegistryEntry &entry);
        /**
         * @brief Unregisters the resource or callback.
         * @param uuid The UUID used by this operation.
         */
        static void unregister(TypedUUID uuid);

        // Abhängigkeitsgraph
        /**
         * @brief Returns the dependencies.
         * @param uuid The UUID used by this operation.
         * @return The result of the operation.
         */
        [[nodiscard]] static const std::vector<AssetDependency> &
        getDependencies(TypedUUID uuid);
        /**
         * @brief Reports whether hard dependency.
         * @param from The from used by this operation.
         * @param to The to used by this operation.
         * @return The result of the operation.
         */
        [[nodiscard]] static bool isHardDependency(TypedUUID from, TypedUUID to);

        // Mod-Overrides
        /**
         * @brief Registers override.
         * @param target The target used by this operation.
         * @param replacementPath The replacement path used by this operation.
         */
        static void registerOverride(TypedUUID target, const std::string &replacementPath);
        /**
         * @brief Clears overrides.
         */
        static void clearOverrides();

        // Scanning/Rebuild (mit Priorität: Mod > Game > Engine)
        /**
         * @brief Scans mount.
         * @param mountName The mount name used by this operation.
         * @param isModMount The is mod mount used by this operation.
         */
        static void scanMount(const std::string &mountName, bool isModMount = false);
        /**
         * @brief Rebuilds from mounts.
         */
        static void rebuildFromMounts();

        // Integrität prüfen (erkennt Zyklen in harthen Abhängigkeiten)
        /**
         * @brief Validates dependency graph.
         * @return The result of the operation.
         */
        [[nodiscard]] static bool validateDependencyGraph();

        // Statistics
        /**
         * @brief Returns the asset count.
         * @return The result of the operation.
         */
        [[nodiscard]] static size_t getAssetCount();
        /**
         * @brief Returns the total asset size.
         * @return The result of the operation.
         */
        [[nodiscard]] static size_t getTotalAssetSize();

      private:
        /** @brief Stores the data for the override. */
        struct Override {
            /** @brief Stores the replacement path. */
            std::string replacementPath;
            /** @brief Stores the priority. */
            int priority; // Höher = höhere Priorität
        };

        /** @brief Stores the entries. */
        static inline std::unordered_map<TypedUUID, AssetRegistryEntry> sEntries;
        /** @brief Stores the overrides. */
        static inline std::unordered_map<TypedUUID, Override> sOverrides;
        /** @brief Stores the mount priorities. */
        static inline std::unordered_map<std::string, int> sMountPriorities;

        /**
         * @brief Detects cycles in the dependency graph.
         * @param start The start used by this operation.
         * @param visited The visited used by this operation.
         * @param inStack The in stack used by this operation.
         * @return The result of the operation.
         */
        [[nodiscard]] static bool detectCycles(TypedUUID start,
                                               std::unordered_set<TypedUUID> &visited,
                                               std::unordered_set<TypedUUID> &inStack);
    };

} // namespace axiom