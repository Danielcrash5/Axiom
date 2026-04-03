#pragma once
#include <string>
#include <memory>
#include <unordered_set>

class Shader {
public:
    virtual ~Shader() = default;

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual void SetFloat(const std::string& name, float value) = 0;

    static std::shared_ptr<Shader> Create(
        const std::string& path,
        const std::unordered_set<std::string>& defines = {}
    );
};