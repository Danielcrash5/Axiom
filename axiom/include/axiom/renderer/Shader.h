#pragma once
#include <memory>
#include <string>

class Shader {
public:
    virtual ~Shader() = default;

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual void SetFloat(const std::string& name, float value) = 0;

    static std::shared_ptr<Shader> Create(const std::string& path);
};