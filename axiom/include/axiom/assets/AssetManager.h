// AssetManager.h
#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>

#include "VFS.h"
#include "axiom/renderer/Texture2D.h"
#include "axiom/renderer/Shader.h"
#include "TextureLoadInfo.h"

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
        static std::shared_ptr<T> LoadAsset(const std::string& virtualPath, Args&&... args);

        // -------- Texture2D Spezialisierung --------
        template<>
        static std::shared_ptr<Texture2D> LoadAsset<Texture2D>(
            const std::string& virtualPath,
            const TextureLoadInfo& info
        ) {
            std::vector<uint8_t> data;
            if (!VFS::ReadFile(virtualPath, data))
                throw std::runtime_error("Failed to read texture asset: " + virtualPath);

            return Texture2D::CreateFromMemory(data, info);
        }

        template<>
        static std::shared_ptr<Texture2D> LoadAsset<Texture2D>(
            const std::string& virtualPath
        ) {
            TextureLoadInfo defaults = TexturePresets::Albedo();
            return LoadAsset<Texture2D>(virtualPath, defaults);
        }

        // -------- Shader Spezialisierung --------
        template<>
        static std::shared_ptr<Shader> LoadAsset<Shader>(
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