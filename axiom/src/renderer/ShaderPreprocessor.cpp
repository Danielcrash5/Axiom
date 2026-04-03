#include "axiom/renderer/ShaderPreprocessor.h"
#include <fstream>
#include <sstream>

static std::string ReadFile(const std::string& path) {
    std::ifstream file(path);
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
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