#include "axiom/renderer/ShaderPreprocessor.h"
#include "axiom/assets/VFS.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>

static bool IsVirtualPath(const std::string& path) {
    return path.find("://") != std::string::npos;
}

static std::string ReadFile(const std::string& path) {
    if (IsVirtualPath(path)) {
        std::vector<uint8_t> data;
        if (!axiom::VFS::ReadFile(path, data))
            throw std::runtime_error("Shader include not found: " + path);
        return std::string(data.begin(), data.end());
    }

    std::ifstream file(path);
    if (!file)
        throw std::runtime_error("Shader include not found: " + path);

    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

static std::string NormalizeIncludePath(std::string path) {
    std::replace(path.begin(), path.end(), '\\', '/');
    std::vector<std::string> segments;
    size_t start = 0;
    bool hasScheme = false;

    if (path.find("://") != std::string::npos) {
        hasScheme = true;
        size_t schemeEnd = path.find("//") + 2;
        segments.push_back(path.substr(0, schemeEnd));
        start = schemeEnd;
    }

    std::string segment;
    for (size_t i = start; i <= path.size(); ++i) {
        if (i == path.size() || path[i] == '/') {
            if (!segment.empty()) {
                if (segment == ".") {
                    // skip
                } else if (segment == ".." && segments.size() > (hasScheme ? 1 : 0)) {
                    segments.pop_back();
                } else {
                    segments.push_back(segment);
                }
                segment.clear();
            }
        } else {
            segment.push_back(path[i]);
        }
    }

    std::string normalized;
    if (!segments.empty()) {
        normalized = segments[0];
        for (size_t i = 1; i < segments.size(); ++i) {
            if (!normalized.empty() && normalized.back() != '/')
                normalized += '/';
            normalized += segments[i];
        }
    }
    return normalized;
}

std::string ShaderPreprocessor::Process(
    const std::string& source,
    const std::string& directory,
    const std::unordered_set<std::string>& defines) {
    std::string result = ResolveIncludes(source, directory);
    return ApplyDefines(defines) + result;
}

std::string ShaderPreprocessor::ResolveIncludes(
    const std::string& source,
    const std::string& directory) {
    std::stringstream input(source);
    std::stringstream output;

    std::string line;
    while (std::getline(input, line)) {
        if (line.find("#include") != std::string::npos) {
            size_t start = line.find("\"") + 1;
            size_t end = line.find("\"", start);

            std::string includePath =
                directory + "/" + line.substr(start, end - start);

            includePath = NormalizeIncludePath(includePath);
            output << ReadFile(includePath) << "\n";
        }
        else {
            output << line << "\n";
        }
    }

    return output.str();
}

std::string ShaderPreprocessor::ApplyDefines(
    const std::unordered_set<std::string>& defines) {
    std::stringstream ss;
    for (auto& d : defines)
        ss << "#define " << d << "\n";
    return ss.str();
}