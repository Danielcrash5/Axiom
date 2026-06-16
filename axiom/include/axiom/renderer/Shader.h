#pragma once
#include <axiom/renderer/RendererRHI.h>
#include <vector>
#include <cstdint>

namespace axiom {

    struct ShaderBinaryDesc {
        ShaderStage stage;
        std::vector<uint32_t> spirvCode; // Der fix und fertig geladene Bytecode
    };

    class Shader {
    public:
        Shader(GraphicsDevice& device, const std::vector<ShaderBinaryDesc>& binaries, const PipelineStateDesc& config);
        ~Shader();

        PipelineHandle get_pipeline_handle() const {
            return m_pipeline;
        }
        const std::vector<ResourceBindingDesc>& get_bindings() const {
            return m_bindings;
        }

    private:
        PipelineHandle m_pipeline;
        std::vector<ResourceBindingDesc> m_bindings;
    };

} // namespace axiom