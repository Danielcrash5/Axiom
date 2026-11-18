/**
 * @file axiom/include/axiom/assets/LoaderRegistry.h
 * @brief Declares the public interface for the loader registry.
 */

#pragma once
#include <axiom/assets/AssetManager.h>

namespace axiom {

    // === Provisorisches Loader-Registrierungs-System ===
    // Später ersetzt durch Reflection-basiertes System.
    // Die Makros können direkt in Asset-Dateien aufgerufen werden.

    namespace detail {
        /**
         * @brief Provides the static loader registration functionality.
         * @tparam T Template type used by this type.
         * @tparam StaticLoaderRegistration Template type used by this type.
         */
        template <typename T>
        class StaticLoaderRegistration {
          public:
            /**
             * @brief Creates an instance of static loader registration.
             * @param typeId The type ID used by this operation.
             * @param loader The loader used by this operation.
             */
            StaticLoaderRegistration(AssetTypeId typeId, IAssetLoader *loader) {
                AssetManager::registerLoader(typeId, loader);
            }
        };
    } // namespace detail

    // Makro für einfache Loader-Registrierung
    // Verwendung:
    //   REGISTER_ASSET_LOADER(TextureAsset, TextureLoader, AssetTypeId::Texture);
    //
    /** @brief Provides the REGISTER_ASSET_LOADER macro. */
    /**
     * @brief Performs the register asset loader operation.
     * @return The result of the operation.
     */
    #define REGISTER_ASSET_LOADER(AssetType, LoaderType, TypeId)                           \
        namespace {                                                                         \
            LoaderType g_##AssetType##Loader;                                              \
            /**
             * @brief Performs the registration operation.
             * @param Loader The loader used by this operation.
             * @return The result of the operation.
             */
            axiom::detail::StaticLoaderRegistration<AssetType> g_##AssetType##Registration( \
                TypeId, &g_##AssetType##Loader);                                           \
        }

    // === Mod-Override Manifest Format ===
    // Mods können eine overrides.json mitbringen, die Original-Assets überschreibt

    /** @brief Stores the data for the mod override entry. */
    struct ModOverrideEntry {
        /** @brief Stores the target UUID. */
        std::string target_uuid;      // UUID des zu überschreibenden Assets
        /** @brief Stores the replacement path. */
        std::string replacement_path; // Mod://pfad zur Replacment-Datei
    };

    /** @brief Stores the data for the mod manifest. */
    struct ModManifest {
        /** @brief Stores the mod name. */
        std::string mod_name;
        /** @brief Stores the version. */
        std::string version;
        /** @brief Stores the author. */
        std::string author;
        /** @brief Stores the description. */
        std::string description;
        /** @brief Stores the overrides. */
        std::vector<ModOverrideEntry> overrides;

        // Lade Manifest aus JSON
        /**
         * @brief Loads from JSON.
         * @param jsonText The JSON text used by this operation.
         * @return The result of the operation.
         */
        static ModManifest loadFromJson(const std::string &jsonText);
        /**
         * @brief Saves to JSON.
         * @param pretty The pretty used by this operation.
         * @return The result of the operation.
         */
        std::string saveToJson(bool pretty = true) const;
    };

    // === Mod-System ===
    /**
     * @brief Processes the mod system system work.
     * @tparam ModSystem Template type used by this type.
     */
    class ModSystem {
      public:
        // Registriere einen Mod (per Mount)
        /**
         * @brief Registers mod.
         * @param modName The mod name used by this operation.
         * @param modPath The mod path used by this operation.
         */
        static void registerMod(const std::string &modName, const std::string &modPath);

        // Lade Manifest eines Mods
        /**
         * @brief Returns the mod manifest.
         * @param modName The mod name used by this operation.
         * @return The result of the operation.
         */
        static ModManifest getModManifest(const std::string &modName);

        // Wende Mod-Overrides auf Registry an
        /**
         * @brief Applies the configured mod overrides.
         * @param modName The mod name used by this operation.
         */
        static void applyModOverrides(const std::string &modName);

        // Entferne alle Overrides eines Mods
        /**
         * @brief Removes mod overrides.
         * @param modName The mod name used by this operation.
         */
        static void removeModOverrides(const std::string &modName);

        // Liste alle registrierten Mods auf
        /**
         * @brief Returns the loaded mods.
         * @return The result of the operation.
         */
        static std::vector<std::string> getLoadedMods();
    };

} // namespace axiom
