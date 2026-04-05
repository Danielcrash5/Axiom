#include "axiom/assets/VFS.h"

#include <fstream>
#include <algorithm>

extern "C" {
#include <compat/unzip.h> // minizip-ng Kompatibilitäts-API
}

namespace axiom {

    std::unordered_map<std::string, VFS::MountPoint> VFS::s_mounts;

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
        MountPoint mp;
        mp.type = type;
        mp.physicalPath = path;

        if (type == MountType::Zip) {
            unzFile zf = unzOpen64(path.c_str());
            if (!zf) {
                // Zip konnte nicht geöffnet werden – Mount wird ignoriert
                return;
            }
            mp.zipHandle = zf;
        }

        s_mounts[root] = mp;
    }

    static bool starts_with(const std::string& s, const std::string& prefix) {
        return s.size() >= prefix.size()
            && std::equal(prefix.begin(), prefix.end(), s.begin());
    }

    bool VFS::ReadFile(const std::string& virtualPath, std::vector<uint8_t>& outData) {
        for (auto& [root, mp] : s_mounts) {
            if (!starts_with(virtualPath, root))
                continue;

            std::string relative = virtualPath.substr(root.size());

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