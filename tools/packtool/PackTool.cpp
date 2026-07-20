#include <axiom/assets/VFS.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

extern "C" {
#include <compat/zip.h> // minizip-ng Kompatibilitäts-API
}

namespace {

constexpr std::string_view InputRoot = "input://";
constexpr std::string_view OutputRoot = "output://";

std::string NormalizePath(std::string path) {
    std::replace(path.begin(), path.end(), '\\', '/');
    return path;
}

std::string ParentPath(const std::string &path) {
    const std::string normalized = NormalizePath(path);
    const size_t separator = normalized.find_last_of('/');
    if (separator == std::string::npos)
        return ".";
    if (separator == 0)
        return "/";
    return normalized.substr(0, separator);
}

std::string ToZipPath(const std::string &virtualPath) {
    std::string zipPath = virtualPath;
    if (zipPath.starts_with(InputRoot))
        zipPath.erase(0, InputRoot.size());
    if (!zipPath.empty() && zipPath.front() == '/')
        zipPath.erase(zipPath.begin());
    return zipPath;
}

static bool addFileToZip(zipFile zf, const std::string &virtualPath) {
    const std::string zipPath = ToZipPath(virtualPath);

    std::vector<uint8_t> data;
    if (!axiom::VFS::ReadFile(virtualPath, data)) {
        std::cerr << "Failed to read file through VFS: " << virtualPath
                  << "\n";
        return false;
    }

    // Nur "stored" (keine Kompression), damit wir keine ZLIB-Abhängigkeit
    // brauchen
    int err =
        zipOpenNewFileInZip64(zf, zipPath.c_str(),
                              nullptr,    // zip_fileinfo
                              nullptr, 0, // extrafield local
                              nullptr, 0, // extrafield global
                              nullptr,    // comment
                              0,          // method (0 = store)
                              0, // level (0 = default / irrelevant bei store)
                              0  // zip64
        );

    if (err != ZIP_OK) {
        std::cerr << "zipOpenNewFileInZip64 failed for " << zipPath
                  << " (err=" << err << ")\n";
        return false;
    }

    size_t offset = 0;
    while (offset < data.size()) {
        const size_t remaining = data.size() - offset;
        const auto chunkSize =
            static_cast<unsigned int>(std::min<size_t>(remaining, 64 * 1024));

        if (zipWriteInFileInZip(zf, data.data() + offset, chunkSize) !=
            ZIP_OK) {
            std::cerr << "zipWriteInFileInZip failed for " << zipPath << "\n";
            zipCloseFileInZip(zf);
            return false;
        }

        offset += chunkSize;
    }

    zipCloseFileInZip(zf);
    return true;
}

} // namespace

int main(int argc, char **argv) {
    axiom::VFS::Init();

    if (argc < 3) {
        std::cout << "Usage:\n"
                  << "  AxiomPackTool <output_pack.AxPack> <input_directory>\n";
        axiom::VFS::Shutdown();
        return 1;
    }

    const std::string outputPack = NormalizePath(argv[1]);
    const std::string inputDir = argv[2];

    if (!axiom::VFS::MountPath(std::string(InputRoot), inputDir) ||
        !axiom::VFS::IsDirectory(std::string(InputRoot))) {
        std::cerr << "Input directory does not exist: " << inputDir << "\n";
        axiom::VFS::Shutdown();
        return 1;
    }

    const std::string outputDir = ParentPath(outputPack);
    if (!axiom::VFS::MountPath(std::string(OutputRoot), outputDir, false) ||
        !axiom::VFS::CreateDirectory(std::string(OutputRoot))) {
        std::cerr << "Failed to create output directory: " << outputDir
                  << "\n";
        axiom::VFS::Shutdown();
        return 1;
    }

    const std::string outputZip = outputPack;
    zipFile zf = zipOpen64(outputZip.c_str(), 0);
    if (!zf) {
        std::cerr << "Failed to open output pack: " << outputPack << "\n";
        axiom::VFS::Shutdown();
        return 1;
    }

    std::vector<std::string> files =
        axiom::VFS::ListFiles(std::string(InputRoot), true);
    std::sort(files.begin(), files.end());

    for (const auto &file : files) {
        if (!addFileToZip(zf, file)) {
            std::cerr << "Failed to add file: " << file << "\n";
            zipClose(zf, nullptr);
            axiom::VFS::Shutdown();
            return 1;
        }
    }

    zipClose(zf, nullptr);
    axiom::VFS::Shutdown();
    std::cout << "Pack created: " << outputZip << "\n";
    return 0;
}
