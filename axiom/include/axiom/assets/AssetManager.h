// AssetManager.h
#pragma once
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "VFS.h"
#include "TextureLoadInfo.h"
#include "axiom/renderer/Shader.h"
#include "axiom/renderer/Texture2D.h"

namespace axiom {

    class AssetManager {
    public:
        template<typename T, typename... Args>
        static std::shared_ptr<T> Get(const std::string& nameOrPath, Args&&... args) {
            std::string virtualPath = ResolveAlias(nameOrPath);

            auto it = s_assets.find(virtualPath);
            if (it != s_assets.end())
                return std::static_pointer_cast<T>(it->second);

            auto asset = LoadAsset<T>(virtualPath, std::forward<Args>(args)...);
            s_assets[virtualPath] = asset;
            return asset;
        }

        static void Alias(const std::string& alias, const std::string& virtualPath) {
            s_aliases[alias] = virtualPath;
        }

    private:
        static std::string ResolveAlias(const std::string& nameOrPath) {
            auto it = s_aliases.find(nameOrPath);
            if (it != s_aliases.end())
                return it->second;
            return nameOrPath;
        }

        template<typename T, typename... Args>
        static std::shared_ptr<T> LoadAsset(const std::string& virtualPath, Args&&... args) {
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

            std::string src(reinterpret_cast<const char*>(data.data()), data.size());
            return Shader::CreateFromMemory(src);
        }

        static inline std::unordered_map<std::string, std::shared_ptr<void>> s_assets;
        static inline std::unordered_map<std::string, std::string> s_aliases;
    };

} // namespace axiom
