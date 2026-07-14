#include "axiom/assets/VFS.h"

#include <algorithm>
#include <filesystem>
#include <fstream>

extern "C" {
#include <compat/unzip.h>
}

namespace fs = std::filesystem;

namespace axiom {

std::unordered_map<std::string, VFS::MountPoint> VFS::s_mounts;

static std::string NormalizePath(std::string path) {
    std::replace(path.begin(), path.end(), '\\', '/');
    while (path.find("//") != std::string::npos &&
           path.find("://") != path.find("//"))
        path.replace(path.find("//"), 2, "/");
    return path;
}

void VFS::Init() { s_mounts.clear(); }

void VFS::Shutdown() {
    for (auto &[root, mp] : s_mounts)
        if (mp.type == MountType::Zip && mp.zipHandle)
            unzClose(reinterpret_cast<unzFile>(mp.zipHandle));
    s_mounts.clear();
}

void VFS::Mount(const std::string &root, const std::string &path,
                MountType type, bool readOnly) {
    MountPoint mp{};
    mp.type = type;
    mp.physicalPath = NormalizePath(path);
    mp.readOnly = readOnly;

    if (type == MountType::Zip) {
        auto zf = unzOpen64(mp.physicalPath.c_str());
        if (!zf)
            return;
        mp.zipHandle = zf;
        mp.readOnly = true;
    }

    s_mounts[NormalizePath(root)] = std::move(mp);
}

void VFS::MountPath(const std::string &root, const std::string &path,
                    bool readOnly) {
    auto p = NormalizePath(path);
    auto lower = p;
    std::ranges::transform(lower, lower.begin(), ::tolower);

    if (lower.ends_with(".zip") || lower.ends_with(".pak") ||
        lower.ends_with(".axpack"))
        Mount(root, p, MountType::Zip, true);
    else
        Mount(root, p, MountType::Directory, readOnly);
}

bool VFS::Unmount(const std::string &root) {
    auto it = s_mounts.find(NormalizePath(root));
    if (it == s_mounts.end())
        return false;

    if (it->second.type == MountType::Zip && it->second.zipHandle)
        unzClose(reinterpret_cast<unzFile>(it->second.zipHandle));

    s_mounts.erase(it);
    return true;
}

bool VFS::ResolvePath(const std::string &virtualPath, ResolvedPath &out) {
    auto path = NormalizePath(virtualPath);

    size_t bestLen = 0;
    MountPoint *bestMount = nullptr;
    std::string bestRoot;

    for (auto &[root, mp] : s_mounts) {
        if (path.starts_with(root) && root.size() > bestLen) {
            bestLen = root.size();
            bestMount = &mp;
            bestRoot = root;
        }
    }

    if (!bestMount)
        return false;

    out.mount = bestMount;
    out.relativePath = path.substr(bestRoot.size());

    if (bestMount->type == MountType::Directory)
        out.physicalPath =
            (fs::path(bestMount->physicalPath) / out.relativePath).string();

    return true;
}

bool VFS::ReadFile(const std::string &virtualPath,
                   std::vector<uint8_t> &outData) {
    ResolvedPath rp;
    if (!ResolvePath(virtualPath, rp))
        return false;

    if (rp.mount->type == MountType::Directory) {
        std::ifstream f(rp.physicalPath, std::ios::binary);
        if (!f)
            return false;

        outData.assign(std::istreambuf_iterator<char>(f), {});
        return true;
    }

    auto zf = reinterpret_cast<unzFile>(rp.mount->zipHandle);

    if (unzLocateFile(zf, rp.relativePath.c_str(), 1) != UNZ_OK)
        return false;

    if (unzOpenCurrentFile(zf) != UNZ_OK)
        return false;

    unz_file_info64 info{};
    unzGetCurrentFileInfo64(zf, &info, nullptr, 0, nullptr, 0, nullptr, 0);

    outData.resize(static_cast<size_t>(info.uncompressed_size));

    int read = unzReadCurrentFile(zf, outData.data(),
                                  static_cast<unsigned int>(outData.size()));

    unzCloseCurrentFile(zf);

    return read >= 0;
}

bool VFS::ReadTextFile(const std::string &virtualPath, std::string &outText) {
    std::vector<uint8_t> data;
    if (!ReadFile(virtualPath, data))
        return false;

    outText.assign(data.begin(), data.end());
    return true;
}

bool VFS::WriteFile(const std::string &virtualPath,
                    const std::vector<uint8_t> &data) {
    ResolvedPath rp;
    if (!ResolvePath(virtualPath, rp))
        return false;

    if (rp.mount->type != MountType::Directory)
        return false;
    if (rp.mount->readOnly)
        return false;

    fs::create_directories(fs::path(rp.physicalPath).parent_path());

    std::ofstream f(rp.physicalPath, std::ios::binary);
    if (!f)
        return false;

    f.write(reinterpret_cast<const char *>(data.data()),
            static_cast<std::streamsize>(data.size()));

    return f.good();
}

bool VFS::WriteTextFile(const std::string &virtualPath,
                        const std::string &text) {
    return WriteFile(virtualPath,
                     std::vector<uint8_t>(text.begin(), text.end()));
}

bool VFS::Exists(const std::string &virtualPath) {
    ResolvedPath rp;
    if (!ResolvePath(virtualPath, rp))
        return false;

    if (rp.mount->type == MountType::Directory)
        return fs::exists(rp.physicalPath);

    auto zf = reinterpret_cast<unzFile>(rp.mount->zipHandle);
    return unzLocateFile(zf, rp.relativePath.c_str(), 1) == UNZ_OK;
}

bool VFS::CreateDirectory(const std::string &virtualPath) {
    ResolvedPath rp;
    if (!ResolvePath(virtualPath, rp))
        return false;

    if (rp.mount->type != MountType::Directory)
        return false;
    if (rp.mount->readOnly)
        return false;

    return fs::create_directories(rp.physicalPath) ||
           fs::exists(rp.physicalPath);
}

std::vector<std::string> VFS::ListFiles(const std::string &virtualPath,
                                        bool recursive) {
    std::vector<std::string> result;

    ResolvedPath rp;
    if (!ResolvePath(virtualPath, rp))
        return result;

    if (rp.mount->type == MountType::Directory) {
        if (!fs::exists(rp.physicalPath))
            return result;

        if (recursive) {
            for (auto &e : fs::recursive_directory_iterator(rp.physicalPath))
                if (e.is_regular_file())
                    result.push_back(e.path().string());
        } else {
            for (auto &e : fs::directory_iterator(rp.physicalPath))
                if (e.is_regular_file())
                    result.push_back(e.path().string());
        }
    } else {
        auto zf = reinterpret_cast<unzFile>(rp.mount->zipHandle);

        if (unzGoToFirstFile(zf) != UNZ_OK)
            return result;

        do {
            char filename[1024]{};
            unzGetCurrentFileInfo64(zf, nullptr, filename, sizeof(filename),
                                    nullptr, 0, nullptr, 0);

            std::string name = filename;

            if (recursive) {
                if (name.starts_with(rp.relativePath))
                    result.push_back(name);
            } else {
                if (name.starts_with(rp.relativePath))
                    result.push_back(name);
            }
        } while (unzGoToNextFile(zf) == UNZ_OK);
    }

    return result;
}

std::vector<uint8_t> VFS::ReadFile(const std::string &virtualPath) {
    std::vector<uint8_t> data;
    ReadFile(virtualPath, data);
    return data;
}

uint64_t VFS::GetFileSize(const std::string &virtualPath) {
    ResolvedPath rp;
    if (!ResolvePath(virtualPath, rp))
        return 0;

    if (rp.mount->type == MountType::Directory) {
        if (!std::filesystem::exists(rp.physicalPath))
            return 0;

        return static_cast<uint64_t>(
            std::filesystem::file_size(rp.physicalPath));
    }

    auto zf = reinterpret_cast<unzFile>(rp.mount->zipHandle);

    if (unzLocateFile(zf, rp.relativePath.c_str(), 1) != UNZ_OK)
        return 0;

    unz_file_info64 info{};
    if (unzGetCurrentFileInfo64(zf, &info, nullptr, 0, nullptr, 0, nullptr,
                                0) != UNZ_OK)
        return 0;

    return static_cast<uint64_t>(info.uncompressed_size);
}

std::filesystem::file_time_type
VFS::GetLastWriteTime(const std::string &virtualPath) {
    ResolvedPath rp;
    if (!ResolvePath(virtualPath, rp))
        return {};

    if (rp.mount->type != MountType::Directory)
        return {};

    if (!std::filesystem::exists(rp.physicalPath))
        return {};

    return std::filesystem::last_write_time(rp.physicalPath);
}

std::vector<std::string> VFS::ListDirectories(const std::string &virtualPath,
                                              bool recursive) {
    std::vector<std::string> result;

    ResolvedPath rp;
    if (!ResolvePath(virtualPath, rp))
        return result;

    if (rp.mount->type != MountType::Directory)
        return result;

    if (!std::filesystem::exists(rp.physicalPath))
        return result;

    if (recursive) {
        for (auto &e :
             std::filesystem::recursive_directory_iterator(rp.physicalPath)) {
            if (e.is_directory())
                result.push_back(e.path().string());
        }
    } else {
        for (auto &e : std::filesystem::directory_iterator(rp.physicalPath)) {
            if (e.is_directory())
                result.push_back(e.path().string());
        }
    }

    return result;
}

} // namespace axiom
