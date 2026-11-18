/**
 * @file axiom/src/assets/VFS.cpp
 * @brief Implements the VFS module.
 */

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

    /**
     * @brief Normalizes a virtual path.
     * @param path The path used by this operation.
     * @return The result of the operation.
     */
    static std::string normalizePath(std::string path) {
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

    /**
     * @brief Converts this value to lower case.
     * @param str The str used by this operation.
     * @return The result of the operation.
     */
    static std::string toLowerCase(const std::string &str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return result;
    }

    // === Implementation ===

    void VFS::init() {
        // Initialisiere Standard-Mounts
        mount("engine", "./axiom/assets", MountType::directory, true, 0);
        mount("game", "./assets", MountType::directory, true, 10);
        mount("temp", "./temp", MountType::directory, false, 20);
        mount("data", "./data", MountType::directory, false, 20);
    }

    void VFS::shutdown() {
        // Unmount alle Mounts (in reverse order)
        std::vector<std::string> mounts;
        for (const auto &[name, _] : sMounts) {
            mounts.push_back(name);
        }
        for (auto it = mounts.rbegin(); it != mounts.rend(); ++it) {
            unmount(*it);
        }
        sFileWatchers.clear();
    }

    bool VFS::mount(const std::string &mountName,
                    const std::string &physicalPath, MountType type,
                    bool readOnly, int priority) {
        if (isMounted(mountName)) {
            std::cerr << "VFS: Mount '" << mountName
                      << "' bereits vorhanden!\n";
            return false;
        }

        std::string normalized = normalizePath(physicalPath);

        if (type == MountType::directory) {
            if (!fs::exists(normalized)) {
                std::cerr << "VFS: Verzeichnis nicht gefunden: " << normalized
                          << "\n";
                return false;
            }
            if (!fs::is_directory(normalized)) {
                std::cerr << "VFS: Ist kein Verzeichnis: " << normalized
                          << "\n";
                return false;
            }
        }
        // TODO: ZIP-Support in AssetPack-Phase

        sMounts[mountName] = {type,     normalized, readOnly,
                               priority, nullptr,    0};
        return true;
    }

    bool VFS::mountPath(const std::string &mountName,
                        const std::string &physicalPath, bool readOnly,
                        int priority, MountType type) {
        std::string normalizedRoot = mountName;
        const size_t schemePos = normalizedRoot.find("://");
        if (schemePos != std::string::npos) {
            normalizedRoot = normalizedRoot.substr(0, schemePos);
        }

        if (normalizedRoot.empty()) {
            return false;
        }

        std::string normalizedPhysical = normalizePath(physicalPath);
        if (type == MountType::directory && !fs::exists(normalizedPhysical)) {
            fs::create_directories(normalizedPhysical);
        }

        return mount(normalizedRoot, normalizedPhysical, type, readOnly,
                     priority);
    }

    bool VFS::unmount(const std::string &mountName) {
        return sMounts.erase(mountName) > 0;
    }

    bool VFS::isMounted(const std::string &mountName) {
        return sMounts.find(mountName) != sMounts.end();
    }

    std::pair<std::string, std::string>
    VFS::splitVirtualPath(const std::string &virtualPath) {
        size_t colonPos = virtualPath.find("://");
        if (colonPos != std::string::npos) {
            return {virtualPath.substr(0, colonPos),
                    virtualPath.substr(colonPos + 3)};
        }
        return {"", ""};
    }

    bool VFS::resolvePath(const std::string &virtualPath, ResolvedPath &out) {
        auto [mountName, relativePath] = splitVirtualPath(virtualPath);

        if (mountName.empty()) { // relativePath.empty() NICHT mehr als Fehler
                                 // behandeln
            std::cerr << "VFS: Ungültiger virtueller Pfad: " << virtualPath
                      << "\n";
            return false;
        }

        auto it = sMounts.find(mountName);
        if (it == sMounts.end()) {
            std::cerr << "VFS: Mount nicht gefunden: " << mountName << "\n";
            return false;
        }

        out.mount = &it->second;
        out.relativePath = relativePath; // kann jetzt "" sein
        out.physicalPath = relativePath.empty()
                               ? it->second.physicalPath
                               : it->second.physicalPath + "/" + relativePath;
        return true;
    }

    bool VFS::readFile(const std::string &virtualPath,
                       std::vector<uint8_t> &outData) {
        ResolvedPath resolved;
        if (!resolvePath(virtualPath, resolved)) {
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

    std::vector<uint8_t> VFS::readFile(const std::string &virtualPath) {
        std::vector<uint8_t> data;
        readFile(virtualPath, data);
        return data;
    }

    bool VFS::readTextFile(const std::string &virtualPath,
                           std::string &outText) {
        std::vector<uint8_t> data;
        if (!readFile(virtualPath, data)) {
            return false;
        }

        outText.assign(data.begin(), data.end());
        return true;
    }

    bool VFS::writeFile(const std::string &virtualPath,
                        const std::vector<uint8_t> &data) {
        ResolvedPath resolved;
        if (!resolvePath(virtualPath, resolved)) {
            return false;
        }

        if (resolved.mount->readOnly) {
            std::cerr << "VFS: Versuch, in Read-Only Mount zu schreiben: "
                      << virtualPath << "\n";
            return false;
        }

        try {
            // Stelle sicher, dass das Verzeichnis existiert
            fs::create_directories(
                fs::path(resolved.physicalPath).parent_path());

            std::ofstream file(resolved.physicalPath, std::ios::binary);
            if (!file) {
                return false;
            }

            if (!data.empty()) {
                file.write(reinterpret_cast<const char *>(data.data()),
                           data.size());
            }

            return file.good();
        } catch (const std::exception &e) {
            std::cerr << "VFS: Fehler beim Schreiben: " << e.what() << "\n";
            return false;
        }
    }

    bool VFS::writeTextFile(const std::string &virtualPath,
                            const std::string &text) {
        std::vector<uint8_t> data(text.begin(), text.end());
        return writeFile(virtualPath, data);
    }

    std::future<std::vector<uint8_t>>
    VFS::readFileAsync(const std::string &virtualPath) {
        return std::async(std::launch::async,
                          [virtualPath]() { return readFile(virtualPath); });
    }

    bool VFS::exists(const std::string &virtualPath) {
        ResolvedPath resolved;
        if (!resolvePath(virtualPath, resolved)) {
            return false;
        }
        return fs::exists(resolved.physicalPath);
    }

    bool VFS::isFile(const std::string &virtualPath) {
        ResolvedPath resolved;
        if (!resolvePath(virtualPath, resolved)) {
            return false;
        }
        return fs::is_regular_file(resolved.physicalPath);
    }

    bool VFS::isDirectory(const std::string &virtualPath) {
        ResolvedPath resolved;
        if (!resolvePath(virtualPath, resolved)) {
            return false;
        }
        return fs::is_directory(resolved.physicalPath);
    }

    bool VFS::createDirectory(const std::string &virtualPath) {
        ResolvedPath resolved;
        if (!resolvePath(virtualPath, resolved)) {
            return false;
        }

        if (resolved.mount->readOnly) {
            std::cerr << "VFS: Versuch, in Read-Only Mount zu schreiben: "
                      << virtualPath << "\n";
            return false;
        }

        try {
            fs::create_directories(resolved.physicalPath);
            return fs::is_directory(resolved.physicalPath);
        } catch (const std::exception &e) {
            std::cerr << "VFS: Fehler beim Erstellen des Verzeichnisses: "
                      << e.what() << "\n";
            return false;
        }
    }

    uint64_t VFS::getFileSize(const std::string &virtualPath) {
        ResolvedPath resolved;
        if (!resolvePath(virtualPath, resolved)) {
            return 0;
        }

        try {
            return fs::file_size(resolved.physicalPath);
        } catch (const std::exception &) {
            return 0;
        }
    }

    std::filesystem::file_time_type
    VFS::getLastWriteTime(const std::string &virtualPath) {
        ResolvedPath resolved;
        if (!resolvePath(virtualPath, resolved)) {
            return {};
        }

        try {
            return fs::last_write_time(resolved.physicalPath);
        } catch (const std::exception &) {
            return {};
        }
    }

    std::vector<std::string> VFS::listFiles(const std::string &virtualPath,
                                            bool recursive) {
        ResolvedPath resolved;
        if (!resolvePath(virtualPath, resolved)) {
            return {};
        }

        std::vector<std::string> files;
        try {
            std::function<void(const fs::path &)> collect =
                [&](const fs::path &path) {
                    std::error_code ec;
                    if (fs::exists(path, ec) && fs::is_directory(path, ec)) {
                        for (const auto &entry :
                             fs::directory_iterator(path, ec)) {
                            if (entry.is_regular_file(ec)) {
                                files.push_back(entry.path().string());
                            } else if (recursive && entry.is_directory(ec)) {
                                collect(entry.path());
                            }
                        }
                    }
                };

            collect(fs::path(resolved.physicalPath));
        } catch (const std::exception &) {
        }

        return files;
    }

    std::vector<std::string>
    VFS::listDirectories(const std::string &virtualPath, bool recursive) {
        ResolvedPath resolved;
        if (!resolvePath(virtualPath, resolved)) {
            return {};
        }

        std::vector<std::string> dirs;
        try {
            std::function<void(const fs::path &)> collect =
                [&](const fs::path &path) {
                    std::error_code ec;
                    if (fs::exists(path, ec) && fs::is_directory(path, ec)) {
                        for (const auto &entry :
                             fs::directory_iterator(path, ec)) {
                            if (entry.is_directory(ec)) {
                                dirs.push_back(entry.path().string());
                                if (recursive) {
                                    collect(entry.path());
                                }
                            }
                        }
                    }
                };

            collect(fs::path(resolved.physicalPath));
        } catch (const std::exception &) {
        }

        return dirs;
    }

    void VFS::watchFile(const std::string &virtualPath,
                        std::function<void()> onChanged) {
        sFileWatchers[virtualPath] = onChanged;
    }

    void VFS::unwatchFile(const std::string &virtualPath) {
        sFileWatchers.erase(virtualPath);
    }

    void VFS::pollFileChanges() {
        for (auto &[virtualPath, callback] : sFileWatchers) {
            ResolvedPath resolved;
            if (!resolvePath(virtualPath, resolved)) {
                continue;
            }

            try {
                auto lastWrite = fs::last_write_time(resolved.physicalPath);
                auto lastCheck = resolved.mount->lastCheck;

                if (lastWrite.time_since_epoch() >
                    std::chrono::nanoseconds(lastCheck)) {
                    callback();
                    resolved.mount->lastCheck =
                        lastWrite.time_since_epoch().count();
                }
            } catch (const std::exception &) {
            }
        }
    }

    std::string VFS::resolvePhysicalPath(const std::string &virtualPath) {
        ResolvedPath resolved;
        if (!resolvePath(virtualPath, resolved)) {
            return "";
        }
        return resolved.physicalPath;
    }

    std::string VFS::resolvePhysicalMountPath(
        const std::string &mountRootHint,
        const std::vector<std::string> &candidatePaths,
        const std::vector<std::string> &commandLineArgs, bool &usedFallback) {
        usedFallback = false;

        for (const auto &arg : commandLineArgs) {
            const std::string prefix = "--asset-root=";
            if (arg.rfind(prefix, 0) == 0) {
                const std::string candidate = arg.substr(prefix.size());
                if (!candidate.empty() && fs::exists(candidate)) {
                    return normalizePath(candidate);
                }
            }
        }

        std::string rootHint = mountRootHint;
        const size_t schemePos = rootHint.find("://");
        if (schemePos != std::string::npos) {
            rootHint = rootHint.substr(schemePos + 3);
        }
        if (!rootHint.empty() && fs::exists(rootHint)) {
            return normalizePath(rootHint);
        }

        for (const auto &candidate : candidatePaths) {
            if (!candidate.empty() && fs::exists(candidate)) {
                return normalizePath(candidate);
            }
        }

        if (!candidatePaths.empty()) {
            usedFallback = true;
            return normalizePath(candidatePaths.front());
        }

        return {};
    }

    bool VFS::isPathValid(const std::string &virtualPath) {
        ResolvedPath resolved;
        return resolvePath(virtualPath, resolved);
    }

} // namespace axiom
