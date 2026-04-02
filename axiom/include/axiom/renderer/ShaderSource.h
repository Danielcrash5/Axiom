#pragma once
#include <unordered_map>
#include <string>
#include "ShaderStage.h"

struct ShaderSource {
    std::unordered_map<ShaderStage, std::string> Sources;
};