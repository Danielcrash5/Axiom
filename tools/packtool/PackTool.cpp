#include <iostream>
#include <filesystem>
#include <fstream>
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
            << "  AxiomPackTool <output_pack.zip> <input_directory>\n";
        return 1;
    }

    std::string outputZip = argv[1];
    fs::path inputDir = fs::path(argv[2]);

    if (!fs::exists(inputDir) || !fs::is_directory(inputDir)) {
        std::cerr << "Input directory does not exist: " << inputDir << "\n";
        return 1;
    }

    zipFile zf = zipOpen64(outputZip.c_str(), 0);
    if (!zf) {
        std::cerr << "Failed to open output zip: " << outputZip << "\n";
        return 1;
    }

    for (auto const& entry : fs::recursive_directory_iterator(inputDir)) {
        if (!entry.is_regular_file())
            continue;

        if (!addFileToZip(zf, inputDir, entry.path())) {
            std::cerr << "Failed to add file: " << entry.path() << "\n";
            zipClose(zf, nullptr);
            return 1;
        }
    }

    zipClose(zf, nullptr);
    std::cout << "Pack created: " << outputZip << "\n";
    return 0;
}