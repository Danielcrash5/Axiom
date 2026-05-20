// AssetManager.h
#pragma once
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "VFS.h"
#include "TextureLoadInfo.h"
#include "axiom/core/UUID.h"
#include "axiom/renderer/Shader.h"
#include "axiom/renderer/Texture2D.h"

namespace axiom {

    class AssetManager {
    public:
        template<typename T, typename... Args>
        static std::shared_ptr<T> Get(const std::string& nameOrPath, Args&&... args) {
            UUID assetId = ResolveAssetId(nameOrPath);

            auto it = s_assets.find(assetId);
            if (it != s_assets.end())
                return std::static_pointer_cast<T>(it->second);

            auto asset = LoadAsset<T>(assetId, std::forward<Args>(args)...);
            s_assets[assetId] = asset;
            return asset;
        }

        template<typename T, typename... Args>
        static std::shared_ptr<T> Get(UUID assetId, Args&&... args) {
            auto it = s_assets.find(assetId);
            if (it != s_assets.end())
                return std::static_pointer_cast<T>(it->second);

            auto asset = LoadAsset<T>(assetId, std::forward<Args>(args)...);
            s_assets[assetId] = asset;
            return asset;
        }

        static void Alias(const std::string& alias, const std::string& virtualPath) {
            s_aliases[NormalizePath(alias)] = NormalizePath(virtualPath);
        }

        static UUID ResolveAssetId(const std::string& nameOrPath) {
            const std::string virtualPath = ResolveVirtualPath(nameOrPath);

            auto it = s_assetIds.find(virtualPath);
            if (it != s_assetIds.end())
                return it->second;

            UUID id{};
            s_assetIds[virtualPath] = id;
            s_assetPaths[id] = virtualPath;
            return id;
        }

    private:
        static std::string ResolveVirtualPath(const std::string& nameOrPath) {
            std::string normalized = NormalizePath(nameOrPath);
            auto it = s_aliases.find(normalized);
            if (it != s_aliases.end())
                return it->second;
            return normalized;
        }

        static std::string NormalizePath(std::string path) {
            std::replace(path.begin(), path.end(), '\\', '/');
            for (size_t i = 1; i < path.size(); ++i) {
                if (path[i] == '/' && path[i - 1] == '/') {
                    path.erase(path.begin() + i);
                    --i;
                }
            }
            return path;
        }

        template<typename T, typename... Args>
        static std::shared_ptr<T> LoadAsset(UUID assetId, Args&&... args) {
            const std::string& virtualPath = s_assetPaths.at(assetId);

            if constexpr (std::is_same_v<T, Texture2D>) {
                if constexpr (sizeof...(Args) == 0) {
                    return LoadTextureAsset(virtualPath, TexturePresets::Albedo());
                } else {
                    static_assert(sizeof...(Args) == 1, "Texture2D assets accept zero or one TextureLoadInfo argument");
                    return LoadTextureAsset(virtualPath, std::forward<Args>(args)...);
                }
            } else if constexpr (std::is_same_v<T, Shader>) {
                static_assert(sizeof...(Args) == 0, "Shader assets do not accept extra arguments");
                return LoadShaderAsset(virtualPath);
            } else {
                static_assert(!sizeof(T), "Unsupported asset type");
            }
        }

        static std::shared_ptr<Texture2D> LoadTextureAsset(
            const std::string& virtualPath,
            const TextureLoadInfo& info
        ) {
            std::vector<uint8_t> data;
            if (!VFS::ReadFile(virtualPath, data))
                throw std::runtime_error("Failed to read texture asset: " + virtualPath);

            return Texture2D::CreateFromMemory(data, info);
        }

        static std::shared_ptr<Shader> LoadShaderAsset(
            const std::string& virtualPath
        ) {
            std::vector<uint8_t> data;
            if (!VFS::ReadFile(virtualPath, data))
                throw std::runtime_error("Failed to read shader asset: " + virtualPath);

            std::string src(data.begin(), data.end());
            return Shader::CreateFromMemory(src, virtualPath);
        }

        static inline std::unordered_map<UUID, std::shared_ptr<void>> s_assets;
        static inline std::unordered_map<std::string, UUID> s_assetIds;
        static inline std::unordered_map<UUID, std::string> s_assetPaths;
        static inline std::unordered_map<std::string, std::string> s_aliases;
    };

} // namespace axiom
