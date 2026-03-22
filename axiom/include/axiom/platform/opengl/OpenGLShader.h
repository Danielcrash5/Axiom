#pragma once

#include "axiom/renderer/Shader.h"
#include <unordered_set>

namespace axiom {

    class OpenGLShader : public Shader {
    public:

        OpenGLShader(const ShaderDesc& desc);
        ~OpenGLShader();

        void Bind() const override;

        const std::vector<ShaderResource>& GetResources() const override {
            return m_Resources;
        }

    private:

        uint32_t m_ProgramID = 0;

        std::vector<ShaderResource> m_Resources;

    private:

        std::string ReadFileWithIncludes(const std::string& path);

        std::string ProcessIncludes(
            const std::string& source,
            const std::string& directory,
            std::unordered_set<std::string>& includedFiles
        );

        uint32_t CompileStage(uint32_t type, const std::string& source);

        void Reflect();

    };

}