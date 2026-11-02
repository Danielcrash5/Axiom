#include <axiom/renderer/ShaderCompiler.h>

#include <axiom/assets/VFS.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

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

        std::string ResolvePhysicalPathOrOriginal(const std::string& path) {
            if (path.find("://") != std::string::npos) {
                if (const auto resolved = axiom::VFS::ResolvePhysicalPath(path);
                    !resolved.empty()) {
                    return resolved;
                }
            }
            return path;
        }

        std::optional<std::filesystem::path>
        ResolveIncludeCandidate(const std::string& requestedSource,
                                const std::string& requestingSource) {
            const std::string normalized = std::string(requestedSource);
            if (normalized.empty()) {
                return std::nullopt;
            }

            std::vector<std::filesystem::path> searchDirs;

            const auto requestPath = std::filesystem::path(
                ResolvePhysicalPathOrOriginal(requestingSource));
            if (!requestPath.empty()) {
                const auto parent = requestPath.parent_path();
                if (!parent.empty()) {
                    searchDirs.push_back(parent);
                }
            }

            const auto requestedPath = std::filesystem::path(
                ResolvePhysicalPathOrOriginal(requestedSource));
            if (requestedPath.is_absolute()) {
                searchDirs.push_back(requestedPath);
            }

            std::filesystem::path direct = requestedSource;
            if (direct.is_absolute()) {
                searchDirs.push_back(direct);
            }

            std::vector<std::filesystem::path> candidates;
            for (const auto& dir : searchDirs) {
                candidates.push_back(dir / requestedSource);
            }
            candidates.push_back(std::filesystem::path(requestedSource));
            candidates.push_back(std::filesystem::path(ResolvePhysicalPathOrOriginal(
                requestedSource)));

            for (const auto& candidate : candidates) {
                if (candidate.empty()) {
                    continue;
                }
                if (std::filesystem::exists(candidate) &&
                    std::filesystem::is_regular_file(candidate)) {
                    return candidate;
                }
            }

            return std::nullopt;
        }

        struct IncludedFile {
            std::string resolvedPath;
            std::string text;
        };

        class VfsIncludeResolver final
            : public shaderc::CompileOptions::IncluderInterface {
        public:
            explicit VfsIncludeResolver(std::string rootPath)
                : m_rootPath(std::move(rootPath)) {}

            shaderc_include_result* GetInclude(const char* requested_source,
                                               shaderc_include_type type,
                                               const char* requesting_source,
                                               size_t include_depth) override {
                (void)type;
                (void)include_depth;

                const std::string requested = requested_source ? requested_source : "";
                const std::string requesting = requesting_source ? requesting_source : "";
                const auto lookup = ResolveIncludeCandidate(requested, requesting);
                if (!lookup) {
                    const std::string message = "Cannot find or open include file: " + requested;
                    auto* error = new shaderc_include_result{"", 0, message.data(),
                                                            message.size(), nullptr};
                    return error;
                }

                std::ifstream stream(lookup->string(), std::ios::binary);
                if (!stream) {
                    const std::string message = "Cannot read include file: " + lookup->string();
                    auto* error = new shaderc_include_result{"", 0, message.data(),
                                                            message.size(), nullptr};
                    return error;
                }

                const std::string content((std::istreambuf_iterator<char>(stream)),
                                          std::istreambuf_iterator<char>());

                auto* included = new IncludedFile{lookup->string(), content};
                auto* result = new shaderc_include_result{
                    included->resolvedPath.data(), included->resolvedPath.size(),
                    included->text.data(), included->text.size(), included};
                return result;
            }

            void ReleaseInclude(shaderc_include_result* data) override {
                if (!data) {
                    return;
                }
                delete static_cast<IncludedFile*>(data->user_data);
                delete data;
            }

        private:
            std::string m_rootPath;
        };
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
        options.SetIncluder(std::make_unique<VfsIncludeResolver>(sourceName));

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
