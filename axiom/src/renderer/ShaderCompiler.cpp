/**
 * @file axiom/src/renderer/ShaderCompiler.cpp
 * @brief Implements the shader compiler module.
 */

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
        /**
         * @brief Converts this value to shaderc kind.
         * @param stage The stage used by this operation.
         * @return The result of the operation.
         */
        shaderc_shader_kind toShadercKind(ShaderStageKind stage) {
            switch (stage) {
            case ShaderStageKind::vertex:
                return shaderc_glsl_vertex_shader;
            case ShaderStageKind::fragment:
                return shaderc_glsl_fragment_shader;
            case ShaderStageKind::compute:
                return shaderc_glsl_compute_shader;
            }
            return shaderc_glsl_vertex_shader;
        }

        /**
         * @brief Resolves physical path or original.
         * @param path The path used by this operation.
         * @return The result of the operation.
         */
        std::string resolvePhysicalPathOrOriginal(const std::string& path) {
            if (path.find("://") != std::string::npos) {
                if (const auto resolved = axiom::VFS::resolvePhysicalPath(path);
                    !resolved.empty()) {
                    return resolved;
                }
            }
            return path;
        }

        /**
         * @brief Resolves include candidate.
         * @param requestedSource The requested source used by this operation.
         * @param requestingSource The requesting source used by this operation.
         * @return The result of the operation.
         */
        std::optional<std::filesystem::path>
        resolveIncludeCandidate(const std::string& requestedSource,
                                const std::string& requestingSource) {
            const std::string normalized = std::string(requestedSource);
            if (normalized.empty()) {
                return std::nullopt;
            }

            std::vector<std::filesystem::path> searchDirs;

            const auto requestPath = std::filesystem::path(
                resolvePhysicalPathOrOriginal(requestingSource));
            if (!requestPath.empty()) {
                const auto parent = requestPath.parent_path();
                if (!parent.empty()) {
                    searchDirs.push_back(parent);
                }
            }

            const auto requestedPath = std::filesystem::path(
                resolvePhysicalPathOrOriginal(requestedSource));
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
            candidates.push_back(std::filesystem::path(resolvePhysicalPathOrOriginal(
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

        /** @brief Stores the data for the included file. */
        struct IncludedFile {
            /** @brief Stores the resolved path. */
            std::string resolvedPath;
            /** @brief Stores the text. */
            std::string text;
        };

        /**
         * @brief Provides the VFS include resolver functionality.
         * @tparam VfsIncludeResolver Template type used by this type.
         */
        class VfsIncludeResolver final
            : public shaderc::CompileOptions::IncluderInterface {
        public:
            explicit VfsIncludeResolver(std::string rootPath)
                : m_rootPath(std::move(rootPath)) {}

            shaderc_include_result* GetInclude(const char* requestedSource,
                                               shaderc_include_type type,
                                               const char* requestingSource,
                                               size_t includeDepth) override {
                (void)type;
                (void)includeDepth;

                const std::string requested = requestedSource ? requestedSource : "";
                const std::string requesting = requestingSource ? requestingSource : "";
                const auto lookup = resolveIncludeCandidate(requested, requesting);
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
    ShaderCompiler::compile(const std::string &source,
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
            source, toShadercKind(stage), sourceName.c_str(), options);

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
