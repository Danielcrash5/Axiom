#pragma once

#include <string>
#include <memory>
#include <vector>

namespace axiom {

    enum class ShaderResourceType {
        Uniform,
        Sampler2D,
        Unknown
    };

    struct ShaderResource {
        std::string name;
        ShaderResourceType type;

        uint32_t location = 0;
        uint32_t binding = 0;
    };

    struct ShaderDesc {
        std::string Vertex;
        std::string Fragment;
        std::string Geometry;
        std::string Compute;
        std::string TessControl;
        std::string TessEvaluation;
    };

    class Shader {
    public:

        virtual ~Shader() = default;

        virtual void Bind() const = 0;

        virtual const std::vector<ShaderResource>& GetResources() const = 0;

        static std::shared_ptr<Shader> Create(const ShaderDesc& desc);

    };

}