#pragma once

enum class ShaderDataType {
    Float, Vec2, Vec3, Vec4
};

static uint32_t GetSize(ShaderDataType type) {
    switch (type) {
    case ShaderDataType::Float: return 4;
    case ShaderDataType::Vec2: return 8;
    case ShaderDataType::Vec3: return 12;
    case ShaderDataType::Vec4: return 16;
    }
    return 0;
}

static uint32_t GetComponentCount(ShaderDataType type) {
    switch (type) {
    case ShaderDataType::Float: return 1;
    case ShaderDataType::Vec2: return 2;
    case ShaderDataType::Vec3: return 3;
    case ShaderDataType::Vec4: return 4;
    }
    return 0;
}