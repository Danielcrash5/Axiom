#include "axiom/assets/VFS.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <unordered_set>

extern "C" {
#include <compat/unzip.h>
}

namespace fs = std::filesystem;

namespace axiom {

    std::unordered_map<std::string, VFS::MountPoint> VFS::s_mounts;

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

    static bool PhysicalPathExists(const fs::path &path) {
        std::error_code ec;
        return fs::exists(path, ec);
    }

    static fs::path ResolveExecutableDirectory(
        const std::vector<std::string> &commandLineArgs) {
        if (commandLineArgs.empty() || commandLineArgs[0].empty())
            return {};

        std::error_code ec;
        fs::path executablePath = commandLineArgs[0];
        if (executablePath.is_relative())
            executablePath = fs::absolute(executablePath, ec);

        if (ec || !executablePath.has_parent_path())
            return {};

        return executablePath.parent_path();
    }

    static std::string JoinVirtualPath(const std::string &root,
                                       const std::string &relativePath) {
        if (relativePath.empty())
            return NormalizePath(root);

        std::string result = NormalizePath(root);
        if (!result.empty() && !result.ends_with('/'))
            result.push_back('/');

        result += NormalizePath(relativePath);
        return result;
    }

    static std::string DirectoryPrefix(std::string path) {
        path = NormalizePath(path);
        if (!path.empty() && !path.ends_with('/'))
            path.push_back('/');
        return path;
    }

    static bool EntryIsUnderDirectory(const std::string &entry,
                                      const std::string &directoryPrefix,
                                      std::string &outRemainder) {
        if (!entry.starts_with(directoryPrefix))
            return false;

        outRemainder = entry.substr(directoryPrefix.size());
        return !outRemainder.empty();
    }

    void VFS::Init() { Shutdown(); }

    void VFS::Shutdown() {
        for (auto &[root, mp] : s_mounts)
            if (mp.type == MountType::Zip && mp.zipHandle)
                unzClose(reinterpret_cast<unzFile>(mp.zipHandle));
        s_mounts.clear();
    }

    bool VFS::Mount(const std::string &root, const std::string &path,
                    MountType type, bool readOnly) {
        if (root.empty() || path.empty())
            return false;

        MountPoint mp{};
        mp.type = type;
        mp.physicalPath = NormalizePath(path);
        mp.readOnly = readOnly;

        if (type == MountType::Directory && readOnly &&
            !PhysicalPathExists(mp.physicalPath))
            return false;

        if (type == MountType::Zip) {
            auto zf = unzOpen64(mp.physicalPath.c_str());
            if (!zf)
                return false;
            mp.zipHandle = zf;
            mp.readOnly = true;
        }

        const std::string normalizedRoot = NormalizePath(root);
        auto existing = s_mounts.find(normalizedRoot);
        if (existing != s_mounts.end() &&
            existing->second.type == MountType::Zip &&
            existing->second.zipHandle) {
            unzClose(reinterpret_cast<unzFile>(existing->second.zipHandle));
        }

        s_mounts[normalizedRoot] = std::move(mp);
        return true;
    }

    bool VFS::MountPath(const std::string &root, const std::string &path,
                        bool readOnly) {
        auto p = NormalizePath(path);
        auto lower = p;
        std::ranges::transform(lower, lower.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });

        if (lower.ends_with(".zip") || lower.ends_with(".pak") ||
            lower.ends_with(".axpack"))
            return Mount(root, p, MountType::Zip, true);
        else
            return Mount(root, p, MountType::Directory, readOnly);
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

        if (out.relativePath.starts_with('/'))
            out.relativePath.erase(out.relativePath.begin());

        out.relativePath = NormalizePath(out.relativePath);

        if (bestMount->type == MountType::Directory)
            out.physicalPath =
                (fs::path(bestMount->physicalPath) / out.relativePath).string();

        return true;
    }

    std::string VFS::ResolvePhysicalMountPath(
        const std::string &configuredPath,
        const std::vector<std::string> &fallbackCandidates,
        const std::vector<std::string> &commandLineArgs, bool &usedFallback) {
        usedFallback = false;

        if (!configuredPath.empty() && PhysicalPathExists(configuredPath))
            return NormalizePath(configuredPath);

        const fs::path executableDir =
            ResolveExecutableDirectory(commandLineArgs);
        if (!configuredPath.empty() && !executableDir.empty()) {
            fs::path configured = configuredPath;
            if (configured.is_relative()) {
                fs::path executableRelativePath = executableDir / configured;
                if (PhysicalPathExists(executableRelativePath))
                    return NormalizePath(executableRelativePath.string());
            }
        }

        for (const auto &candidate : fallbackCandidates) {
            if (PhysicalPathExists(candidate)) {
                usedFallback = true;
                return NormalizePath(candidate);
            }
        }

        return NormalizePath(configuredPath);
    }

    bool VFS::ReadFile(const std::string &virtualPath,
                       std::vector<uint8_t> &outData) {
        ResolvedPath rp;
        if (!ResolvePath(virtualPath, rp))
            return false;

        if (rp.mount->type == MountType::Directory) {
            std::ifstream f(rp.physicalPath,
                            std::ios::binary | std::ios::ate);
            if (!f)
                return false;

            const std::streampos end = f.tellg();
            if (end < 0)
                return false;

            outData.resize(static_cast<size_t>(end));
            f.seekg(0, std::ios::beg);

            if (!outData.empty() &&
                !f.read(reinterpret_cast<char *>(outData.data()),
                        static_cast<std::streamsize>(outData.size())))
                return false;

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

        size_t offset = 0;
        while (offset < outData.size()) {
            const size_t remaining = outData.size() - offset;
            const auto chunkSize = static_cast<unsigned int>(
                std::min<size_t>(remaining, 64 * 1024));
            const int read = unzReadCurrentFile(zf, outData.data() + offset,
                                                chunkSize);

            if (read < 0) {
                unzCloseCurrentFile(zf);
                outData.clear();
                return false;
            }

            if (read == 0)
                break;

            offset += static_cast<size_t>(read);
        }

        unzCloseCurrentFile(zf);

        if (offset != outData.size()) {
            outData.clear();
            return false;
        }

        return true;
    }

    bool VFS::ReadTextFile(const std::string &virtualPath,
                           std::string &outText) {
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

        std::error_code ec;
        const fs::path parentPath = fs::path(rp.physicalPath).parent_path();
        if (!parentPath.empty()) {
            fs::create_directories(parentPath, ec);
            if (ec)
                return false;
        }

        std::ofstream f(rp.physicalPath, std::ios::binary);
        if (!f)
            return false;

        if (!data.empty())
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
            return PhysicalPathExists(rp.physicalPath);

        if (rp.relativePath.empty())
            return true;

        auto zf = reinterpret_cast<unzFile>(rp.mount->zipHandle);
        if (unzLocateFile(zf, rp.relativePath.c_str(), 1) == UNZ_OK)
            return true;

        const std::string directoryEntry = DirectoryPrefix(rp.relativePath);
        if (unzLocateFile(zf, directoryEntry.c_str(), 1) == UNZ_OK)
            return true;

        if (unzGoToFirstFile(zf) != UNZ_OK)
            return false;

        do {
            char filename[1024]{};
            unzGetCurrentFileInfo64(zf, nullptr, filename, sizeof(filename),
                                    nullptr, 0, nullptr, 0);

            if (NormalizePath(filename).starts_with(directoryEntry))
                return true;
        } while (unzGoToNextFile(zf) == UNZ_OK);

        return false;
    }

    bool VFS::IsFile(const std::string &virtualPath) {
        ResolvedPath rp;
        if (!ResolvePath(virtualPath, rp))
            return false;

        if (rp.mount->type == MountType::Directory) {
            std::error_code ec;
            return fs::is_regular_file(rp.physicalPath, ec);
        }

        if (rp.relativePath.empty() || rp.relativePath.ends_with('/'))
            return false;

        auto zf = reinterpret_cast<unzFile>(rp.mount->zipHandle);
        return unzLocateFile(zf, rp.relativePath.c_str(), 1) == UNZ_OK;
    }

    bool VFS::IsDirectory(const std::string &virtualPath) {
        ResolvedPath rp;
        if (!ResolvePath(virtualPath, rp))
            return false;

        if (rp.mount->type == MountType::Directory) {
            std::error_code ec;
            return fs::is_directory(rp.physicalPath, ec);
        }

        if (rp.relativePath.empty())
            return true;

        auto zf = reinterpret_cast<unzFile>(rp.mount->zipHandle);
        const std::string directoryEntry = DirectoryPrefix(rp.relativePath);
        if (unzLocateFile(zf, directoryEntry.c_str(), 1) == UNZ_OK)
            return true;

        if (unzGoToFirstFile(zf) != UNZ_OK)
            return false;

        do {
            char filename[1024]{};
            unzGetCurrentFileInfo64(zf, nullptr, filename, sizeof(filename),
                                    nullptr, 0, nullptr, 0);

            if (NormalizePath(filename).starts_with(directoryEntry))
                return true;
        } while (unzGoToNextFile(zf) == UNZ_OK);

        return false;
    }

    bool VFS::CreateDirectory(const std::string &virtualPath) {
        ResolvedPath rp;
        if (!ResolvePath(virtualPath, rp))
            return false;

        if (rp.mount->type != MountType::Directory)
            return false;
        if (rp.mount->readOnly)
            return false;

        std::error_code ec;
        return fs::create_directories(rp.physicalPath, ec) ||
               (!ec && PhysicalPathExists(rp.physicalPath));
    }

    std::vector<std::string> VFS::ListFiles(const std::string &virtualPath,
                                            bool recursive) {
        std::vector<std::string> result;

        ResolvedPath rp;
        if (!ResolvePath(virtualPath, rp))
            return result;

        if (rp.mount->type == MountType::Directory) {
            if (!PhysicalPathExists(rp.physicalPath))
                return result;

            std::error_code ec;
            if (recursive) {
                for (auto &e :
                     fs::recursive_directory_iterator(rp.physicalPath, ec)) {
                    if (ec)
                        break;
                    if (!e.is_regular_file())
                        continue;

                    const fs::path rel =
                        fs::relative(e.path(), rp.physicalPath, ec);
                    if (ec)
                        continue;

                    result.push_back(JoinVirtualPath(
                        virtualPath, NormalizePath(rel.generic_string())));
                }
            } else {
                for (auto &e : fs::directory_iterator(rp.physicalPath, ec)) {
                    if (ec)
                        break;
                    if (e.is_regular_file())
                        result.push_back(JoinVirtualPath(
                            virtualPath, e.path().filename().generic_string()));
                }
            }
        } else {
            auto zf = reinterpret_cast<unzFile>(rp.mount->zipHandle);

            if (unzGoToFirstFile(zf) != UNZ_OK)
                return result;

            do {
                char filename[1024]{};
                unzGetCurrentFileInfo64(zf, nullptr, filename, sizeof(filename),
                                        nullptr, 0, nullptr, 0);

                const std::string name = NormalizePath(filename);
                if (name.empty() || name.ends_with('/'))
                    continue;

                std::string remainder;
                if (!EntryIsUnderDirectory(name, DirectoryPrefix(rp.relativePath),
                                           remainder))
                    continue;

                if (!recursive && remainder.find('/') != std::string::npos)
                    continue;

                result.push_back(
                    JoinVirtualPath(virtualPath, NormalizePath(remainder)));
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
            if (!PhysicalPathExists(rp.physicalPath))
                return 0;

            std::error_code ec;
            const auto size = fs::file_size(rp.physicalPath, ec);
            return ec ? 0 : static_cast<uint64_t>(size);
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

        if (!PhysicalPathExists(rp.physicalPath))
            return {};

        std::error_code ec;
        const auto lastWriteTime = fs::last_write_time(rp.physicalPath, ec);
        return ec ? std::filesystem::file_time_type{} : lastWriteTime;
    }

    std::vector<std::string>
    VFS::ListDirectories(const std::string &virtualPath, bool recursive) {
        std::vector<std::string> result;

        ResolvedPath rp;
        if (!ResolvePath(virtualPath, rp))
            return result;

        if (rp.mount->type == MountType::Directory) {
            if (!PhysicalPathExists(rp.physicalPath))
                return result;

            std::error_code ec;
            if (recursive) {
                for (auto &e :
                     fs::recursive_directory_iterator(rp.physicalPath, ec)) {
                    if (ec)
                        break;
                    if (!e.is_directory())
                        continue;

                    const fs::path rel =
                        fs::relative(e.path(), rp.physicalPath, ec);
                    if (ec)
                        continue;

                    result.push_back(JoinVirtualPath(
                        virtualPath, NormalizePath(rel.generic_string())));
                }
            } else {
                for (auto &e : fs::directory_iterator(rp.physicalPath, ec)) {
                    if (ec)
                        break;
                    if (e.is_directory())
                        result.push_back(JoinVirtualPath(
                            virtualPath, e.path().filename().generic_string()));
                }
            }

            return result;
        }

        auto zf = reinterpret_cast<unzFile>(rp.mount->zipHandle);

        if (unzGoToFirstFile(zf) != UNZ_OK)
            return result;

        std::unordered_set<std::string> directories;
        do {
            char filename[1024]{};
            unzGetCurrentFileInfo64(zf, nullptr, filename, sizeof(filename),
                                    nullptr, 0, nullptr, 0);

            const std::string name = NormalizePath(filename);
            std::string remainder;
            if (!EntryIsUnderDirectory(name, DirectoryPrefix(rp.relativePath),
                                       remainder))
                continue;

            size_t slash = remainder.find('/');
            if (slash == std::string::npos)
                continue;

            while (slash != std::string::npos) {
                directories.insert(remainder.substr(0, slash));
                if (!recursive)
                    break;

                slash = remainder.find('/', slash + 1);
            }
        } while (unzGoToNextFile(zf) == UNZ_OK);

        result.reserve(directories.size());
        for (const auto &directory : directories)
            result.push_back(JoinVirtualPath(virtualPath, directory));

        std::ranges::sort(result);
        return result;
    }

} // namespace axiom
