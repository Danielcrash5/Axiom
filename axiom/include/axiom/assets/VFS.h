
#pragma once
#include <cstdint>
#include <filesystem>
#include <functional>
#include <future>
#include <string>
#include <unordered_map>
#include <vector>

namespace axiom {

    using VfsReadCallback = std::function<void(const std::vector<uint8_t> &data, bool success)>;

    class VFS {
      public:
        enum class MountType { Directory, Zip, AssetPack };

        static void Init();
        static void Shutdown();

        // Mount-Verwaltung
        // Feste Mounts: engine, game, temp, data
        // Mod-Mounts: dynamisch, mit Priorität
        static bool Mount(const std::string &mountName, const std::string &physicalPath,
                          MountType type = MountType::Directory, bool readOnly = true,
                          int priority = 0);
        static bool MountPath(const std::string &mountName,
                              const std::string &physicalPath,
                              bool readOnly = true, int priority = 0,
                              MountType type = MountType::Directory);
        static bool Unmount(const std::string &mountName);
        static bool IsMounted(const std::string &mountName);

        // Virtuelle Pfade sind IMMER mount-qualifiziert: "MountName://pfad"
        // z.B. "Data://saves/save1.json", "Game://textures/wood.png"

        // === Synchrone APIs ===
        static bool ReadFile(const std::string &virtualPath, std::vector<uint8_t> &outData);
        static std::vector<uint8_t> ReadFile(const std::string &virtualPath);
        static bool ReadTextFile(const std::string &virtualPath, std::string &outText);

        static bool WriteFile(const std::string &virtualPath, const std::vector<uint8_t> &data);
        static bool WriteTextFile(const std::string &virtualPath, const std::string &text);

        // === Asynchrone APIs ===
        // Lädt eine Datei async, ruft Callback mit Daten auf
        static std::future<std::vector<uint8_t>> ReadFileAsync(const std::string &virtualPath);

        // === Datei-Operationen ===
        static bool Exists(const std::string &virtualPath);
        static bool IsFile(const std::string &virtualPath);
        static bool IsDirectory(const std::string &virtualPath);
        static bool CreateDirectory(const std::string &virtualPath);

        static uint64_t GetFileSize(const std::string &virtualPath);
        static std::filesystem::file_time_type GetLastWriteTime(const std::string &virtualPath);

        // === Verzeichnis-Listing ===
        static std::vector<std::string> ListFiles(const std::string &virtualPath,
                                                   bool recursive = false);
        static std::vector<std::string> ListDirectories(const std::string &virtualPath,
                                                        bool recursive = false);

        // === Editor Hot-Reload Support ===
        static void WatchFile(const std::string &virtualPath,
                              std::function<void()> onChanged);
        static void UnwatchFile(const std::string &virtualPath);
        static void PollFileChanges(); // Ruft in Editor-Loop auf

        // === Hilfer ===
        static std::string ResolvePhysicalPath(const std::string &virtualPath);
        static std::string ResolvePhysicalMountPath(
            const std::string &mountRootHint,
            const std::vector<std::string> &candidatePaths,
            const std::vector<std::string> &commandLineArgs, bool &usedFallback);
        static bool IsPathValid(const std::string &virtualPath);

      private:
        struct MountPoint {
            MountType type;
            std::string physicalPath;
            bool readOnly;
            int priority;       // Höher = höhere Priorität bei Kollisionen
            void *zipHandle;    // ZIP-Handle, falls Zip-Mount
            uint64_t lastCheck; // Für File-Watcher
        };

        struct ResolvedPath {
            MountPoint *mount;
            std::string relativePath;
            std::string physicalPath;
        };

        static bool ResolvePath(const std::string &virtualPath, ResolvedPath &out);
        static std::pair<std::string, std::string> SplitVirtualPath(
            const std::string &virtualPath);

        static inline std::unordered_map<std::string, MountPoint> s_mounts;
        static inline std::unordered_map<std::string, std::function<void()>> s_fileWatchers;
    };

} // namespace axiom
