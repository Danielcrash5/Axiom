#include <axiom/assets/VFS.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_set>

namespace fs = std::filesystem;

namespace axiom {

    // === Helper Functions ===

    static std::string NormalizePath(std::string path) {
        std::replace(path.begin(), path.end(), '\\', '/');

        size_t pos = 0;
        while ((pos = path.find("//", pos)) != std::string::npos) {
            if (pos > 0 && path[pos - 1] == ':') {
                pos += 2;
                continue;
            }
            path.replace(pos, 2, "/");
        }

        return path;
    }

    static std::string ToLowerCase(const std::string &str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return result;
    }

    // === Implementation ===

    void VFS::Init() {
        // Initialisiere Standard-Mounts
        Mount("engine", "./axiom/assets", MountType::Directory, true, 0);
        Mount("game", "./assets", MountType::Directory, true, 10);
        Mount("temp", "./temp", MountType::Directory, false, 20);
        Mount("data", "./data", MountType::Directory, false, 20);
    }

    void VFS::Shutdown() {
        // Unmount alle Mounts (in reverse order)
        std::vector<std::string> mounts;
        for (const auto &[name, _] : s_mounts) {
            mounts.push_back(name);
        }
        for (auto it = mounts.rbegin(); it != mounts.rend(); ++it) {
            Unmount(*it);
        }
        s_fileWatchers.clear();
    }

    bool VFS::Mount(const std::string &mountName, const std::string &physicalPath,
                     MountType type, bool readOnly, int priority) {
        if (IsMounted(mountName)) {
            std::cerr << "VFS: Mount '" << mountName << "' bereits vorhanden!\n";
            return false;
        }

        std::string normalized = NormalizePath(physicalPath);

        if (type == MountType::Directory) {
            if (!fs::exists(normalized)) {
                std::cerr << "VFS: Verzeichnis nicht gefunden: " << normalized << "\n";
                return false;
            }
            if (!fs::is_directory(normalized)) {
                std::cerr << "VFS: Ist kein Verzeichnis: " << normalized << "\n";
                return false;
            }
        }
        // TODO: ZIP-Support in AssetPack-Phase

        s_mounts[mountName] = {type, normalized, readOnly, priority, nullptr, 0};
        return true;
    }

    bool VFS::Unmount(const std::string &mountName) {
        return s_mounts.erase(mountName) > 0;
    }

    bool VFS::IsMounted(const std::string &mountName) {
        return s_mounts.find(mountName) != s_mounts.end();
    }

    std::pair<std::string, std::string> VFS::SplitVirtualPath(const std::string &virtualPath) {
        size_t colonPos = virtualPath.find("://");
        if (colonPos != std::string::npos) {
            return {virtualPath.substr(0, colonPos), virtualPath.substr(colonPos + 3)};
        }
        return {"", ""};
    }

    bool VFS::ResolvePath(const std::string &virtualPath, ResolvedPath &out) {
        auto [mountName, relativePath] = SplitVirtualPath(virtualPath);

        if (mountName.empty() || relativePath.empty()) {
            std::cerr << "VFS: Ungültiger virtueller Pfad: " << virtualPath << "\n";
            return false;
        }

        auto it = s_mounts.find(mountName);
        if (it == s_mounts.end()) {
            std::cerr << "VFS: Mount nicht gefunden: " << mountName << "\n";
            return false;
        }

        out.mount = &it->second;
        out.relativePath = NormalizePath(relativePath);
        out.physicalPath = NormalizePath(it->second.physicalPath + "/" + out.relativePath);

        return true;
    }

    bool VFS::ReadFile(const std::string &virtualPath, std::vector<uint8_t> &outData) {
        ResolvedPath resolved;
        if (!ResolvePath(virtualPath, resolved)) {
            return false;
        }

        try {
            std::ifstream file(resolved.physicalPath, std::ios::binary);
            if (!file) {
                return false;
            }

            file.seekg(0, std::ios::end);
            size_t size = file.tellg();
            file.seekg(0, std::ios::beg);

            outData.resize(size);
            if (size > 0) {
                file.read(reinterpret_cast<char *>(outData.data()), size);
            }

            return file.good();
        } catch (const std::exception &e) {
            std::cerr << "VFS: Fehler beim Lesen: " << e.what() << "\n";
            return false;
        }
    }

    std::vector<uint8_t> VFS::ReadFile(const std::string &virtualPath) {
        std::vector<uint8_t> data;
        ReadFile(virtualPath, data);
        return data;
    }

    bool VFS::ReadTextFile(const std::string &virtualPath, std::string &outText) {
        std::vector<uint8_t> data;
        if (!ReadFile(virtualPath, data)) {
            return false;
        }

        outText.assign(data.begin(), data.end());
        return true;
    }

    bool VFS::WriteFile(const std::string &virtualPath, const std::vector<uint8_t> &data) {
        ResolvedPath resolved;
        if (!ResolvePath(virtualPath, resolved)) {
            return false;
        }

        if (resolved.mount->readOnly) {
            std::cerr << "VFS: Versuch, in Read-Only Mount zu schreiben: " << virtualPath
                      << "\n";
            return false;
        }

        try {
            // Stelle sicher, dass das Verzeichnis existiert
            fs::create_directories(fs::path(resolved.physicalPath).parent_path());

            std::ofstream file(resolved.physicalPath, std::ios::binary);
            if (!file) {
                return false;
            }

            if (!data.empty()) {
                file.write(reinterpret_cast<const char *>(data.data()), data.size());
            }

            return file.good();
        } catch (const std::exception &e) {
            std::cerr << "VFS: Fehler beim Schreiben: " << e.what() << "\n";
            return false;
        }
    }

    bool VFS::WriteTextFile(const std::string &virtualPath, const std::string &text) {
        std::vector<uint8_t> data(text.begin(), text.end());
        return WriteFile(virtualPath, data);
    }

    std::future<std::vector<uint8_t>> VFS::ReadFileAsync(const std::string &virtualPath) {
        return std::async(std::launch::async, [virtualPath]() { return ReadFile(virtualPath); });
    }

    bool VFS::Exists(const std::string &virtualPath) {
        ResolvedPath resolved;
        if (!ResolvePath(virtualPath, resolved)) {
            return false;
        }
        return fs::exists(resolved.physicalPath);
    }

    bool VFS::IsFile(const std::string &virtualPath) {
        ResolvedPath resolved;
        if (!ResolvePath(virtualPath, resolved)) {
            return false;
        }
        return fs::is_regular_file(resolved.physicalPath);
    }

    bool VFS::IsDirectory(const std::string &virtualPath) {
        ResolvedPath resolved;
        if (!ResolvePath(virtualPath, resolved)) {
            return false;
        }
        return fs::is_directory(resolved.physicalPath);
    }

    bool VFS::CreateDirectory(const std::string &virtualPath) {
        ResolvedPath resolved;
        if (!ResolvePath(virtualPath, resolved)) {
            return false;
        }

        if (resolved.mount->readOnly) {
            std::cerr << "VFS: Versuch, in Read-Only Mount zu schreiben: " << virtualPath
                      << "\n";
            return false;
        }

        try {
            return fs::create_directories(resolved.physicalPath);
        } catch (const std::exception &e) {
            std::cerr << "VFS: Fehler beim Erstellen des Verzeichnisses: " << e.what()
                      << "\n";
            return false;
        }
    }

    uint64_t VFS::GetFileSize(const std::string &virtualPath) {
        ResolvedPath resolved;
        if (!ResolvePath(virtualPath, resolved)) {
            return 0;
        }

        try {
            return fs::file_size(resolved.physicalPath);
        } catch (const std::exception &) {
            return 0;
        }
    }

    std::filesystem::file_time_type VFS::GetLastWriteTime(const std::string &virtualPath) {
        ResolvedPath resolved;
        if (!ResolvePath(virtualPath, resolved)) {
            return {};
        }

        try {
            return fs::last_write_time(resolved.physicalPath);
        } catch (const std::exception &) {
            return {};
        }
    }

    std::vector<std::string> VFS::ListFiles(const std::string &virtualPath, bool recursive) {
        ResolvedPath resolved;
        if (!ResolvePath(virtualPath, resolved)) {
            return {};
        }

        std::vector<std::string> files;
        try {
            auto options = recursive ? fs::directory_options::recursive
                                     : fs::directory_options::none;
            for (const auto &entry : fs::directory_iterator(resolved.physicalPath, options)) {
                if (entry.is_regular_file()) {
                    files.push_back(entry.path().string());
                }
            }
        } catch (const std::exception &) {
        }

        return files;
    }

    std::vector<std::string> VFS::ListDirectories(const std::string &virtualPath,
                                                   bool recursive) {
        ResolvedPath resolved;
        if (!ResolvePath(virtualPath, resolved)) {
            return {};
        }

        std::vector<std::string> dirs;
        try {
            auto options = recursive ? fs::directory_options::recursive
                                     : fs::directory_options::none;
            for (const auto &entry : fs::directory_iterator(resolved.physicalPath, options)) {
                if (entry.is_directory()) {
                    dirs.push_back(entry.path().string());
                }
            }
        } catch (const std::exception &) {
        }

        return dirs;
    }

    void VFS::WatchFile(const std::string &virtualPath, std::function<void()> onChanged) {
        s_fileWatchers[virtualPath] = onChanged;
    }

    void VFS::UnwatchFile(const std::string &virtualPath) {
        s_fileWatchers.erase(virtualPath);
    }

    void VFS::PollFileChanges() {
        for (auto &[virtualPath, callback] : s_fileWatchers) {
            ResolvedPath resolved;
            if (!ResolvePath(virtualPath, resolved)) {
                continue;
            }

            try {
                auto lastWrite = fs::last_write_time(resolved.physicalPath);
                auto lastCheck = resolved.mount->lastCheck;

                if (lastWrite.time_since_epoch() > std::chrono::nanoseconds(lastCheck)) {
                    callback();
                    resolved.mount->lastCheck =
                        lastWrite.time_since_epoch().count();
                }
            } catch (const std::exception &) {
            }
        }
    }

    std::string VFS::ResolvePhysicalPath(const std::string &virtualPath) {
        ResolvedPath resolved;
        if (!ResolvePath(virtualPath, resolved)) {
            return "";
        }
        return resolved.physicalPath;
    }

    bool VFS::IsPathValid(const std::string &virtualPath) {
        ResolvedPath resolved;
        return ResolvePath(virtualPath, resolved);
    }

} // namespace axiom
