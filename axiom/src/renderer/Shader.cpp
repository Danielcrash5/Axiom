#include <axiom/renderer/Shader.h>

namespace axiom {

    Shader::Shader(GraphicsDevice& device, const std::vector<ShaderBinaryDesc>& binaries, const PipelineStateDesc& config) {
        PipelineStateDesc finalDesc = config;
        m_bindings = config.reflectedBindings;

        finalDesc.type = PipelineType::Graphics;
        finalDesc.stages.clear();

        // Wir mappen die bereits fertigen Binaries direkt in die Pipeline-Beschreibung
        for (const auto& binary : binaries) {
            ShaderModuleDesc modDesc {
                .stage = binary.stage,
                .spirvCode = binary.spirvCode,
                .entryPoint = "main" // Standardmäßig main, außer du konfigurierst es in der .shadercfg
            };
            finalDesc.stages.push_back(modDesc);

            if (binary.stage == ShaderStage::Compute) {
                finalDesc.type = PipelineType::Compute;
            }
        }

        // Direkt an Vulkan übergeben zum Erstellen der Pipeline
        m_pipeline = device.create_pipeline(finalDesc);
    }

    Shader::~Shader() {
    }

} // namespace axiom