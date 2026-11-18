/**
 * @file axiom/include/axiom/assets/VFS.h
 * @brief Declares the public interface for the VFS.
 */


#pragma once
#include <cstdint>
#include <filesystem>
#include <functional>
#include <future>
#include <string>
#include <unordered_map>
#include <vector>

namespace axiom {

    /**
     * @brief Provides the VFS read callback type alias.
     */
    using VfsReadCallback = std::function<void(const std::vector<uint8_t> &data, bool success)>;

    /**
     * @brief Resolves virtual paths and performs mounted file operations.
     * @tparam VFS Template type used by this type.
     */
    class VFS {
      public:
        /**
         * @brief Defines the available mount type values.
         * @tparam MountType Template type used by this type.
         */
        enum class MountType { directory, zip, assetPack };

        /**
         * @brief Initializes the subsystem.
         */
        static void init();
        /**
         * @brief Shuts down the subsystem.
         */
        static void shutdown();

        // Mount-Verwaltung
        // Feste Mounts: engine, game, temp, data
        // Mod-Mounts: dynamisch, mit Priorität
        /**
         * @brief Mounts the virtual file system source.
         * @param mountName The mount name used by this operation.
         * @param physicalPath The physical path used by this operation.
         * @param type The type used by this operation.
         * @param readOnly The read only used by this operation.
         * @param priority The priority used by this operation.
         * @return The result of the operation.
         */
        static bool mount(const std::string &mountName, const std::string &physicalPath,
                          MountType type = MountType::directory, bool readOnly = true,
                          int priority = 0);
        /**
         * @brief Mounts path.
         * @param mountName The mount name used by this operation.
         * @param physicalPath The physical path used by this operation.
         * @param readOnly The read only used by this operation.
         * @param priority The priority used by this operation.
         * @param type The type used by this operation.
         * @return The result of the operation.
         */
        static bool mountPath(const std::string &mountName,
                              const std::string &physicalPath,
                              bool readOnly = true, int priority = 0,
                              MountType type = MountType::directory);
        /**
         * @brief Unmounts the virtual file system source.
         * @param mountName The mount name used by this operation.
         * @return The result of the operation.
         */
        static bool unmount(const std::string &mountName);
        /**
         * @brief Reports whether mounted.
         * @param mountName The mount name used by this operation.
         * @return The result of the operation.
         */
        static bool isMounted(const std::string &mountName);

        // Virtuelle Pfade sind IMMER mount-qualifiziert: "MountName://pfad"
        // z.B. "Data://saves/save1.json", "Game://textures/wood.png"

        // === Synchrone APIs ===
        /**
         * @brief Reads file.
         * @param virtualPath The virtual path used by this operation.
         * @param[out] outData Receives the data value.
         * @return The result of the operation.
         */
        static bool readFile(const std::string &virtualPath, std::vector<uint8_t> &outData);
        /**
         * @brief Reads file.
         * @param virtualPath The virtual path used by this operation.
         * @return The result of the operation.
         */
        static std::vector<uint8_t> readFile(const std::string &virtualPath);
        /**
         * @brief Reads text file.
         * @param virtualPath The virtual path used by this operation.
         * @param[out] outText Receives the text value.
         * @return The result of the operation.
         */
        static bool readTextFile(const std::string &virtualPath, std::string &outText);

        /**
         * @brief Writes file.
         * @param virtualPath The virtual path used by this operation.
         * @param data The data used by this operation.
         * @return The result of the operation.
         */
        static bool writeFile(const std::string &virtualPath, const std::vector<uint8_t> &data);
        /**
         * @brief Writes text file.
         * @param virtualPath The virtual path used by this operation.
         * @param text The text used by this operation.
         * @return The result of the operation.
         */
        static bool writeTextFile(const std::string &virtualPath, const std::string &text);

        // === Asynchrone APIs ===
        // Lädt eine Datei async, ruft Callback mit Daten auf
        /**
         * @brief Reads file async.
         * @param virtualPath The virtual path used by this operation.
         * @return The result of the operation.
         */
        static std::future<std::vector<uint8_t>> readFileAsync(const std::string &virtualPath);

        // === Datei-Operationen ===
        /**
         * @brief Reports whether the requested resource exists.
         * @param virtualPath The virtual path used by this operation.
         * @return The result of the operation.
         */
        static bool exists(const std::string &virtualPath);
        /**
         * @brief Reports whether file.
         * @param virtualPath The virtual path used by this operation.
         * @return The result of the operation.
         */
        static bool isFile(const std::string &virtualPath);
        /**
         * @brief Reports whether directory.
         * @param virtualPath The virtual path used by this operation.
         * @return The result of the operation.
         */
        static bool isDirectory(const std::string &virtualPath);
        /**
         * @brief Creates directory.
         * @param virtualPath The virtual path used by this operation.
         * @return The result of the operation.
         */
        static bool createDirectory(const std::string &virtualPath);

        /**
         * @brief Returns the file size.
         * @param virtualPath The virtual path used by this operation.
         * @return The result of the operation.
         */
        static uint64_t getFileSize(const std::string &virtualPath);
        /**
         * @brief Returns the last write time.
         * @param virtualPath The virtual path used by this operation.
         * @return The result of the operation.
         */
        static std::filesystem::file_time_type getLastWriteTime(const std::string &virtualPath);

        // === Verzeichnis-Listing ===
        /**
         * @brief Lists files in the requested virtual directory.
         * @param virtualPath The virtual path used by this operation.
         * @param recursive The recursive used by this operation.
         * @return The result of the operation.
         */
        static std::vector<std::string> listFiles(const std::string &virtualPath,
                                                   bool recursive = false);
        /**
         * @brief Lists directories in the requested virtual directory.
         * @param virtualPath The virtual path used by this operation.
         * @param recursive The recursive used by this operation.
         * @return The result of the operation.
         */
        static std::vector<std::string> listDirectories(const std::string &virtualPath,
                                                        bool recursive = false);

        // === Editor Hot-Reload Support ===
        /**
         * @brief Starts watching a file for changes.
         * @param virtualPath The virtual path used by this operation.
         * @param onChanged The on changed used by this operation.
         */
        static void watchFile(const std::string &virtualPath,
                              std::function<void()> onChanged);
        /**
         * @brief Stops watching a file for changes.
         * @param virtualPath The virtual path used by this operation.
         */
        static void unwatchFile(const std::string &virtualPath);
        /**
         * @brief Collects pending file change notifications.
         */
        static void pollFileChanges(); // Ruft in Editor-Loop auf

        // === Hilfer ===
        /**
         * @brief Resolves physical path.
         * @param virtualPath The virtual path used by this operation.
         * @return The result of the operation.
         */
        static std::string resolvePhysicalPath(const std::string &virtualPath);
        /**
         * @brief Resolves physical mount path.
         * @param mountRootHint The mount root hint used by this operation.
         * @param candidatePaths The candidate paths used by this operation.
         * @param commandLineArgs The command line args used by this operation.
         * @param usedFallback The used fallback used by this operation.
         * @return The result of the operation.
         */
        static std::string resolvePhysicalMountPath(
            const std::string &mountRootHint,
            const std::vector<std::string> &candidatePaths,
            const std::vector<std::string> &commandLineArgs, bool &usedFallback);
        /**
         * @brief Reports whether path valid.
         * @param virtualPath The virtual path used by this operation.
         * @return The result of the operation.
         */
        static bool isPathValid(const std::string &virtualPath);

      private:
        /** @brief Stores the data for the mount point. */
        struct MountPoint {
            /** @brief Stores the type. */
            MountType type;
            /** @brief Stores the physical path. */
            std::string physicalPath;
            /** @brief Stores the read only. */
            bool readOnly;
            /** @brief Stores the priority. */
            int priority;       // Höher = höhere Priorität bei Kollisionen
            /** @brief Stores the zip handle. */
            void *zipHandle;    // ZIP-Handle, falls Zip-Mount
            /** @brief Stores the last check. */
            uint64_t lastCheck; // Für File-Watcher
        };

        /** @brief Stores the data for the resolved path. */
        struct ResolvedPath {
            /** @brief Stores the mount. */
            MountPoint *mount;
            /** @brief Stores the relative path. */
            std::string relativePath;
            /** @brief Stores the physical path. */
            std::string physicalPath;
        };

        /**
         * @brief Resolves path.
         * @param virtualPath The virtual path used by this operation.
         * @param[out] out Receives the out value.
         * @return The result of the operation.
         */
        static bool resolvePath(const std::string &virtualPath, ResolvedPath &out);
        /**
         * @brief Splits a virtual path into its components.
         * @param virtualPath The virtual path used by this operation.
         * @return The result of the operation.
         */
        static std::pair<std::string, std::string> splitVirtualPath(
            const std::string &virtualPath);

        /** @brief Stores the mounts. */
        static inline std::unordered_map<std::string, MountPoint> sMounts;
        static inline std::unordered_map<std::string, std::function<void()>> sFileWatchers;
    };

} // namespace axiom
