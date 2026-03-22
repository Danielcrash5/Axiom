#include "axiom/renderer/Shader.h"
#include "axiom/platform/opengl/OpenGLShader.h"

namespace axiom {

    std::shared_ptr<Shader> Shader::Create(const ShaderDesc& desc) {
        return std::make_shared<OpenGLShader>(desc);
    }

}