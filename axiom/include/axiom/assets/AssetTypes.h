#pragma once
#include <axiom/assets/AssetManager.h>
#include <axiom/assets/Serializers.h>
#include <glm/glm.hpp>
#include <vector>

namespace axiom {

    // === TextureAsset - Eager geladen ===
    struct TextureAsset {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t channels = 4; // RGBA = 4
        std::vector<uint8_t> pixelData;

        void Serialize(ISerializer &s) {
            s.Value("width", width);
            s.Value("height", height);
            s.Value("channels", channels);
            uint32_t dataSize = pixelData.size();
            s.Value("dataSize", dataSize);
            if (s.IsReading()) {
                pixelData.resize(dataSize);
            }
        }
    };

    class TextureLoader : public IAssetLoader {
      public:
        void *Load(const std::vector<uint8_t> &data, size_t &outSizeBytes) override;
        void *Reload(void *existing, const std::vector<uint8_t> &newData,
                     size_t &outSizeBytes) override;
        void Unload(void *data) override;
    };

    // === MeshAsset - Eager geladen ===
    struct MeshAsset {
        struct Vertex {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec2 texCoord;
        };

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        void Serialize(ISerializer &s) {
            uint32_t vertexCount = vertices.size();
            s.Value("vertexCount", vertexCount);
            if (s.IsReading()) {
                vertices.resize(vertexCount);
            }

            uint32_t indexCount = indices.size();
            s.Value("indexCount", indexCount);
            if (s.IsReading()) {
                indices.resize(indexCount);
            }
        }
    };

    class MeshLoader : public IAssetLoader {
      public:
        void *Load(const std::vector<uint8_t> &data, size_t &outSizeBytes) override;
        void *Reload(void *existing, const std::vector<uint8_t> &newData,
                     size_t &outSizeBytes) override;
        void Unload(void *data) override;
    };

    // === MaterialAsset - Eager geladen ===
    struct MaterialAsset {
        std::string name;
        glm::vec4 albedoColor{1.0f};
        float metallic = 0.0f;
        float roughness = 1.0f;
        TypedUUID albedoTextureId;   // Soft-Referenz
        TypedUUID normalTextureId;   // Soft-Referenz
        TypedUUID metallicTextureId; // Soft-Referenz

        void Serialize(ISerializer &s) {
            s.Value("name", name);
            s.Value("metallic", metallic);
            s.Value("roughness", roughness);
            // Farben-Serialisierung...
            // Texture-UUIDs als Strings...
        }
    };

    class MaterialLoader : public IAssetLoader {
      public:
        void *Load(const std::vector<uint8_t> &data, size_t &outSizeBytes) override;
        void *Reload(void *existing, const std::vector<uint8_t> &newData,
                     size_t &outSizeBytes) override;
        void Unload(void *data) override;
    };

    // === SceneAsset - Eager geladen, enthält EnTT-Registry Dump ===
    struct SceneAsset {
        std::string name;
        std::vector<uint8_t> registryDump; // Serialisierter EnTT-Registry

        void Serialize(ISerializer &s) {
            s.Value("name", name);
            uint32_t dumpSize = registryDump.size();
            s.Value("registryDumpSize", dumpSize);
            if (s.IsReading()) {
                registryDump.resize(dumpSize);
            }
        }
    };

    class SceneLoader : public IAssetLoader {
      public:
        void *Load(const std::vector<uint8_t> &data, size_t &outSizeBytes) override;
        void *Reload(void *existing, const std::vector<uint8_t> &newData,
                     size_t &outSizeBytes) override;
        void Unload(void *data) override;
    };

    // === PrefabAsset - Eager geladen, Single-Entity-Vorlage ===
    struct PrefabAsset {
        std::string name;
        std::vector<uint8_t> entityDump; // Serialisierte Entity + Komponenten
        std::vector<AssetDependency> componentDependencies;

        void Serialize(ISerializer &s) {
            s.Value("name", name);
            uint32_t dumpSize = entityDump.size();
            s.Value("entityDumpSize", dumpSize);
            if (s.IsReading()) {
                entityDump.resize(dumpSize);
            }
        }
    };

    class PrefabLoader : public IAssetLoader {
      public:
        void *Load(const std::vector<uint8_t> &data, size_t &outSizeBytes) override;
        void *Reload(void *existing, const std::vector<uint8_t> &newData,
                     size_t &outSizeBytes) override;
        void Unload(void *data) override;
    };

    // === SoundEffectAsset - Eager geladen ===
    struct SoundEffectAsset {
        uint32_t sampleRate = 44100;
        uint32_t channels = 2;
        std::vector<int16_t> samples; // PCM-Daten

        void Serialize(ISerializer &s) {
            s.Value("sampleRate", sampleRate);
            s.Value("channels", channels);
            uint32_t sampleCount = samples.size();
            s.Value("sampleCount", sampleCount);
            if (s.IsReading()) {
                samples.resize(sampleCount);
            }
        }
    };

    class SoundEffectLoader : public IAssetLoader {
      public:
        void *Load(const std::vector<uint8_t> &data, size_t &outSizeBytes) override;
        void *Reload(void *existing, const std::vector<uint8_t> &newData,
                     size_t &outSizeBytes) override;
        void Unload(void *data) override;
    };

} // namespace axiom
