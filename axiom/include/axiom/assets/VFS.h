
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

namespace axiom {

    class VFS {
    public:
        enum class MountType {
            Directory, Zip
        };

        static void Init();
        static void Shutdown();

        static void Mount(const std::string& root, const std::string& path,
                          MountType type, bool readOnly = true);
        static void MountPath(const std::string& root, const std::string& path,
                              bool readOnly = true);
        static bool Unmount(const std::string& root);

        static bool ReadFile(const std::string& virtualPath, std::vector<uint8_t>& outData);
        static std::vector<uint8_t> ReadFile(const std::string& virtualPath);

        static bool ReadTextFile(const std::string& virtualPath, std::string& outText);

        static bool WriteFile(const std::string& virtualPath, const std::vector<uint8_t>& data);
        static bool WriteTextFile(const std::string& virtualPath, const std::string& text);

        static bool Exists(const std::string& virtualPath);
        static bool CreateDirectory(const std::string& virtualPath);

        static uint64_t GetFileSize(const std::string& virtualPath);
        static std::filesystem::file_time_type GetLastWriteTime(const std::string& virtualPath);

        static std::vector<std::string> ListFiles(const std::string& virtualPath, bool recursive = false);
        static std::vector<std::string> ListDirectories(const std::string& virtualPath, bool recursive = false);

    private:
        struct MountPoint {
            MountType type;
            std::string physicalPath;
            bool readOnly = true;
            void* zipHandle = nullptr;
        };

        struct ResolvedPath {
            MountPoint* mount = nullptr;
            std::string relativePath;
            std::string physicalPath;
        };

        static bool ResolvePath(const std::string& virtualPath, ResolvedPath& out);
        static std::unordered_map<std::string, MountPoint> s_mounts;
    };

}
