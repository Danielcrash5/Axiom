#include <iostream>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <vector>
#include <string>

extern "C" {
#include <compat/zip.h> // minizip-ng Kompatibilitäts-API
}

namespace fs = std::filesystem;

static bool addFileToZip(zipFile zf, const fs::path& root, const fs::path& filePath) {
    fs::path relPath = fs::relative(filePath, root);
    std::string zipPath = relPath.generic_string(); // immer '/'

    std::ifstream in(filePath, std::ios::binary);
    if (!in) {
        std::cerr << "Failed to open file: " << filePath << "\n";
        return false;
    }

    // Nur "stored" (keine Kompression), damit wir keine ZLIB-Abhängigkeit brauchen
    int err = zipOpenNewFileInZip64(
        zf,
        zipPath.c_str(),
        nullptr,      // zip_fileinfo
        nullptr, 0,   // extrafield local
        nullptr, 0,   // extrafield global
        nullptr,      // comment
        0,            // method (0 = store)
        0,            // level (0 = default / irrelevant bei store)
        0             // zip64
    );

    if (err != ZIP_OK) {
        std::cerr << "zipOpenNewFileInZip64 failed for " << zipPath << " (err=" << err << ")\n";
        return false;
    }

    std::vector<char> buffer(64 * 1024);
    while (in) {
        in.read(buffer.data(), buffer.size());
        std::streamsize got = in.gcount();
        if (got <= 0)
            break;

        if (zipWriteInFileInZip(zf, buffer.data(), static_cast<unsigned int>(got)) != ZIP_OK) {
            std::cerr << "zipWriteInFileInZip failed for " << zipPath << "\n";
            zipCloseFileInZip(zf);
            return false;
        }
    }

    zipCloseFileInZip(zf);
    return true;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cout << "Usage:\n"
            << "  AxiomPackTool <output_pack.AxPack> <input_directory>\n";
        return 1;
    }

    fs::path outputPack = fs::path(argv[1]);
    fs::path inputDir = fs::path(argv[2]);

    if (!fs::exists(inputDir) || !fs::is_directory(inputDir)) {
        std::cerr << "Input directory does not exist: " << inputDir << "\n";
        return 1;
    }

    std::error_code ec;
    if (outputPack.has_parent_path()) {
        fs::create_directories(outputPack.parent_path(), ec);
        if (ec) {
            std::cerr << "Failed to create output directory: " << outputPack.parent_path() << "\n";
            return 1;
        }
    }

    const std::string outputZip = outputPack.string();
    zipFile zf = zipOpen64(outputZip.c_str(), 0);
    if (!zf) {
        std::cerr << "Failed to open output pack: " << outputPack << "\n";
        return 1;
    }

    std::vector<fs::path> files;
    for (auto const& entry : fs::recursive_directory_iterator(inputDir)) {
        if (!entry.is_regular_file())
            continue;
        files.push_back(entry.path());
    }

    std::sort(files.begin(), files.end());

    for (const auto& file : files) {
        if (!addFileToZip(zf, inputDir, file)) {
            std::cerr << "Failed to add file: " << file << "\n";
            zipClose(zf, nullptr);
            return 1;
        }
    }

    zipClose(zf, nullptr);
    std::cout << "Pack created: " << outputZip << "\n";
    return 0;
}
