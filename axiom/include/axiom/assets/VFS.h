#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace axiom {

    class VFS {
    public:
        enum class MountType {
            Directory,
            Zip
        };

        // Initialisierung / Aufräumen
        static void Init();
        static void Shutdown();

        // Mountet ein Verzeichnis oder ein Zip unter einem virtuellen Root, z.B. "engine/", "game/"
        static void Mount(const std::string& root, const std::string& path, MountType type);

        // Liest eine Datei in einen Byte-Buffer
        static bool ReadFile(const std::string& virtualPath, std::vector<uint8_t>& outData);

    private:
        struct MountPoint {
            MountType type;
            std::string physicalPath; // Directory: Ordnerpfad, Zip: Pfad zur Zip-Datei
            void* zipHandle = nullptr; // unzFile (als void*)
        };

        static std::unordered_map<std::string, MountPoint> s_mounts;
    };

} // namespace axiom