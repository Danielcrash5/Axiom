#pragma once
#include <axiom/assets/AssetManager.h>

namespace axiom {

    // === Provisorisches Loader-Registrierungs-System ===
    // Später ersetzt durch Reflection-basiertes System.
    // Die Makros können direkt in Asset-Dateien aufgerufen werden.

    namespace detail {
        template <typename T>
        class StaticLoaderRegistration {
          public:
            StaticLoaderRegistration(AssetTypeId typeId, IAssetLoader *loader) {
                AssetManager::RegisterLoader(typeId, loader);
            }
        };
    } // namespace detail

    // Makro für einfache Loader-Registrierung
    // Verwendung:
    //   REGISTER_ASSET_LOADER(TextureAsset, TextureLoader, AssetTypeId::Texture);
    //
    #define REGISTER_ASSET_LOADER(AssetType, LoaderType, TypeId)                           \
        namespace {                                                                         \
            LoaderType g_##AssetType##Loader;                                              \
            axiom::detail::StaticLoaderRegistration<AssetType> g_##AssetType##Registration( \
                TypeId, &g_##AssetType##Loader);                                           \
        }

    // === Mod-Override Manifest Format ===
    // Mods können eine overrides.json mitbringen, die Original-Assets überschreibt

    struct ModOverrideEntry {
        std::string target_uuid;      // UUID des zu überschreibenden Assets
        std::string replacement_path; // Mod://pfad zur Replacment-Datei
    };

    struct ModManifest {
        std::string mod_name;
        std::string version;
        std::string author;
        std::string description;
        std::vector<ModOverrideEntry> overrides;

        // Lade Manifest aus JSON
        static ModManifest LoadFromJson(const std::string &jsonText);
        std::string SaveToJson(bool pretty = true) const;
    };

    // === Mod-System ===
    class ModSystem {
      public:
        // Registriere einen Mod (per Mount)
        static void RegisterMod(const std::string &modName, const std::string &modPath);

        // Lade Manifest eines Mods
        static ModManifest GetModManifest(const std::string &modName);

        // Wende Mod-Overrides auf Registry an
        static void ApplyModOverrides(const std::string &modName);

        // Entferne alle Overrides eines Mods
        static void RemoveModOverrides(const std::string &modName);

        // Liste alle registrierten Mods auf
        static std::vector<std::string> GetLoadedMods();
    };

} // namespace axiom
