#pragma once
#include <string>
#include <unordered_set>

class ShaderPreprocessor {
public:
    static std::string Process(
        const std::string& source,
        const std::string& directory,
        const std::unordered_set<std::string>& defines
    );

private:
    static std::string ResolveIncludes(
        const std::string& source,
        const std::string& directory
    );

    static std::string ApplyDefines(
        const std::unordered_set<std::string>& defines
    );
};