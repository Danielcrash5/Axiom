#include "axiom/assets/VFS.h"

#include <fstream>
#include <algorithm>
#include <cctype>

extern "C" {
#include <compat/unzip.h> // minizip-ng Kompatibilitäts-API
}

namespace axiom {

    std::unordered_map<std::string, VFS::MountPoint> VFS::s_mounts;

    static std::string NormalizePath(std::string path) {
        std::replace(path.begin(), path.end(), '\\', '/');
        while (path.find("//") != std::string::npos)
            path.erase(path.find("//"), 1);
        return path;
    }

    static bool ends_with_ignore_case(const std::string& value, const std::string& suffix) {
        if (value.size() < suffix.size())
            return false;

        size_t offset = value.size() - suffix.size();
        for (size_t i = 0; i < suffix.size(); ++i) {
            if (std::tolower(static_cast<unsigned char>(value[offset + i])) != std::tolower(static_cast<unsigned char>(suffix[i])))
                return false;
        }
        return true;
    }

    void VFS::Init() {
        s_mounts.clear();
    }

    void VFS::Shutdown() {
        for (auto& [root, mp] : s_mounts) {
            if (mp.type == MountType::Zip && mp.zipHandle) {
                unzClose(reinterpret_cast<unzFile>(mp.zipHandle));
                mp.zipHandle = nullptr;
            }
        }
        s_mounts.clear();
    }

    void VFS::Mount(const std::string& root, const std::string& path, MountType type) {
        std::string normalizedRoot = NormalizePath(root);
        std::string normalizedPath = NormalizePath(path);

        MountPoint mp;
        mp.type = type;
        mp.physicalPath = normalizedPath;

        if (type == MountType::Zip) {
            unzFile zf = unzOpen64(normalizedPath.c_str());
            if (!zf) {
                // Zip konnte nicht geöffnet werden – Mount wird ignoriert
                return;
            }
            mp.zipHandle = zf;
        }

        s_mounts[normalizedRoot] = mp;
    }

    void VFS::MountPath(const std::string& root, const std::string& path) {
        std::string normalizedPath = NormalizePath(path);
        if (ends_with_ignore_case(normalizedPath, ".zip") || ends_with_ignore_case(normalizedPath, ".pak") || ends_with_ignore_case(normalizedPath, ".apack")) {
            Mount(root, normalizedPath, MountType::Zip);
        } else {
            Mount(root, normalizedPath, MountType::Directory);
        }
    }

    static bool starts_with(const std::string& s, const std::string& prefix) {
        return s.size() >= prefix.size()
            && std::equal(prefix.begin(), prefix.end(), s.begin());
    }

    bool VFS::ReadFile(const std::string& virtualPath, std::vector<uint8_t>& outData) {
        std::string normalizedPath = NormalizePath(virtualPath);

        for (auto& [root, mp] : s_mounts) {
            if (!starts_with(normalizedPath, root))
                continue;

            std::string relative = normalizedPath.substr(root.size());

            if (mp.type == MountType::Directory) {
                std::string fullPath = mp.physicalPath;
                if (!fullPath.empty() && fullPath.back() != '/' && fullPath.back() != '\\')
                    fullPath += '/';
                fullPath += relative;

                std::ifstream file(fullPath, std::ios::binary);
                if (!file)
                    return false;

                outData.assign(std::istreambuf_iterator<char>(file),
                               std::istreambuf_iterator<char>());
                return true;
            }

            if (mp.type == MountType::Zip && mp.zipHandle) {
                unzFile zf = reinterpret_cast<unzFile>(mp.zipHandle);

                // Datei im Zip suchen (case-insensitive = 1)
                if (unzLocateFile(zf, relative.c_str(), 1) != UNZ_OK)
                    return false;

                if (unzOpenCurrentFile(zf) != UNZ_OK)
                    return false;

                unz_file_info64 info {};
                if (unzGetCurrentFileInfo64(zf, &info, nullptr, 0, nullptr, 0, nullptr, 0) != UNZ_OK) {
                    unzCloseCurrentFile(zf);
                    return false;
                }

                outData.resize(static_cast<size_t>(info.uncompressed_size));
                uint8_t* dst = outData.data();
                size_t remaining = outData.size();

                while (remaining > 0) {
                    int read = unzReadCurrentFile(zf, dst, static_cast<unsigned int>(remaining));
                    if (read < 0) {
                        unzCloseCurrentFile(zf);
                        return false;
                    }
                    if (read == 0)
                        break;
                    dst += read;
                    remaining -= static_cast<size_t>(read);
                }

                unzCloseCurrentFile(zf);
                return remaining == 0;
            }
        }

        return false;
    }

} // namespace axiom