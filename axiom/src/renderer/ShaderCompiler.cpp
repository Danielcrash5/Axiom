#include <axiom/renderer/ShaderCompiler.h>

#include <shaderc/shaderc.hpp>

namespace axiom::renderer {

    namespace {
        shaderc_shader_kind ToShadercKind(ShaderStageKind stage) {
            switch (stage) {
            case ShaderStageKind::Vertex:
                return shaderc_glsl_vertex_shader;
            case ShaderStageKind::Fragment:
                return shaderc_glsl_fragment_shader;
            case ShaderStageKind::Compute:
                return shaderc_glsl_compute_shader;
            }
            return shaderc_glsl_vertex_shader;
        }
    } // namespace

    ShaderCompiler::Result
    ShaderCompiler::Compile(const std::string &source,
                            const std::string &sourceName,
                            ShaderStageKind stage) const {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        // Explizit setzen - ohne das nimmt shaderc Default-Annahmen an, die
        // nicht zu unserem Vulkan-1.3-Backend passen (z.B. #version-
        // Handling und Spec-Constant-Defaults unterscheiden sich je nach
        // Ziel-Environment).
        options.SetTargetEnvironment(shaderc_target_env_vulkan,
                                     shaderc_env_version_vulkan_1_3);
        options.SetOptimizationLevel(shaderc_optimization_level_performance);

        shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(
            source, ToShadercKind(stage), sourceName.c_str(), options);

        Result result;
        if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
            result.errors = module.GetErrorMessage();
            return result;
        }
        result.spirv.assign(module.cbegin(), module.cend());
        if (!module.GetErrorMessage().empty()) {
            // shaderc schreibt Warnings auch bei Erfolg in GetErrorMessage()
            // (kein separates GetWarningMessage()) - deshalb hier, nicht im
            // Fehlerfall oben.
            result.warnings = module.GetErrorMessage();
        }
        return result;
    }

} // namespace axiom::renderer
