/**
 * @file axiom/include/axiom/assets/AssetTypes.h
 * @brief Declares the public interface for the asset types.
 */

#pragma once
#include <axiom/assets/AssetManager.h>
#include <axiom/serialization/Serializers.h>
#include <glm/glm.hpp>
#include <vector>

namespace axiom {

    // === TextureAsset - Eager geladen ===
    /** @brief Stores the data for the texture asset. */
    struct TextureAsset {
        /** @brief Stores the width. */
        uint32_t width = 0;
        /** @brief Stores the height. */
        uint32_t height = 0;
        /** @brief Stores the channels. */
        uint32_t channels = 4; // RGBA = 4
        /** @brief Stores the pixel data. */
        std::vector<uint8_t> pixelData;

        /**
         * @brief Serializes the object.
         * @tparam S Template type used by this operation.
         * @param s The s used by this operation.
         */
        template <typename S> void serialize(S &s) {
            s.value("width", width);
            s.value("height", height);
            s.value("channels", channels);
            uint32_t dataSize = pixelData.size();
            s.value("dataSize", dataSize);
            if (s.isReading()) {
                pixelData.resize(dataSize);
            }
        }
    };

    /**
     * @brief Provides the texture loader functionality.
     * @tparam TextureLoader Template type used by this type.
     */
    class TextureLoader : public IAssetLoader {
      public:
        /**
         * @brief Loads the requested resource.
         * @param data The data used by this operation.
         * @param[out] outSizeBytes Receives the output size in bytes.
         * @return The result of the operation.
         */
        void *load(const std::vector<uint8_t> &data, size_t &outSizeBytes) override;
        /**
         * @brief Reloads the requested resource.
         * @param existing The existing used by this operation.
         * @param newData The new data used by this operation.
         * @param[out] outSizeBytes Receives the output size in bytes.
         * @return The result of the operation.
         */
        void *reload(void *existing, const std::vector<uint8_t> &newData,
                     size_t &outSizeBytes) override;
        /**
         * @brief Releases the loaded resource.
         * @param data The data used by this operation.
         */
        void unload(void *data) override;
    };

    // === MeshAsset - Eager geladen ===
    /** @brief Stores the data for the mesh asset. */
    struct MeshAsset {
        /** @brief Stores the data for the vertex. */
        struct Vertex {
            /** @brief Stores the position. */
            glm::vec3 position;
            /** @brief Stores the normal. */
            glm::vec3 normal;
            /** @brief Stores the tex coord. */
            glm::vec2 texCoord;
        };

        /** @brief Stores the vertices. */
        std::vector<Vertex> vertices;
        /** @brief Stores the indices. */
        std::vector<uint32_t> indices;

        /**
         * @brief Serializes the object.
         * @tparam S Template type used by this operation.
         * @param s The s used by this operation.
         */
        template <typename S> void serialize(S &s) {
            uint32_t vertexCount = vertices.size();
            s.value("vertexCount", vertexCount);
            if (s.isReading()) {
                vertices.resize(vertexCount);
            }

            uint32_t indexCount = indices.size();
            s.value("indexCount", indexCount);
            if (s.isReading()) {
                indices.resize(indexCount);
            }
        }
    };

    /**
     * @brief Provides the mesh loader functionality.
     * @tparam MeshLoader Template type used by this type.
     */
    class MeshLoader : public IAssetLoader {
      public:
        /**
         * @brief Loads the requested resource.
         * @param data The data used by this operation.
         * @param[out] outSizeBytes Receives the output size in bytes.
         * @return The result of the operation.
         */
        void *load(const std::vector<uint8_t> &data, size_t &outSizeBytes) override;
        /**
         * @brief Reloads the requested resource.
         * @param existing The existing used by this operation.
         * @param newData The new data used by this operation.
         * @param[out] outSizeBytes Receives the output size in bytes.
         * @return The result of the operation.
         */
        void *reload(void *existing, const std::vector<uint8_t> &newData,
                     size_t &outSizeBytes) override;
        /**
         * @brief Releases the loaded resource.
         * @param data The data used by this operation.
         */
        void unload(void *data) override;
    };

    // === MaterialAsset - Eager geladen ===
    /** @brief Stores the data for the material asset. */
    struct MaterialAsset {
        /** @brief Stores the name. */
        std::string name;
        /** @brief Stores the albedo color. */
        glm::vec4 albedoColor{1.0f};
        /** @brief Stores the metallic. */
        float metallic = 0.0f;
        /** @brief Stores the roughness. */
        float roughness = 1.0f;
        /** @brief Stores the albedo texture ID. */
        TypedUUID albedoTextureId;   // Hart-Referenz
        /** @brief Stores the normal texture ID. */
        TypedUUID normalTextureId;   // Hart-Referenz
        /** @brief Stores the metallic texture ID. */
        TypedUUID metallicTextureId; // Hart-Referenz

        /**
         * @brief Serializes the object.
         * @tparam S Template type used by this operation.
         * @param s The s used by this operation.
         */
        template <typename S> void serialize(S &s) {
            s.value("name", name);
            s.value("albedoColor", albedoColor);
            s.value("metallic", metallic);
            s.value("roughness", roughness);
            s.value("albedoTextureId", albedoTextureId);
            s.value("normalTextureId", normalTextureId);
            s.value("metallicTextureId", metallicTextureId);
        }
    };

    /**
     * @brief Provides the material loader functionality.
     * @tparam MaterialLoader Template type used by this type.
     */
    class MaterialLoader : public IAssetLoader {
      public:
        /**
         * @brief Loads the requested resource.
         * @param data The data used by this operation.
         * @param[out] outSizeBytes Receives the output size in bytes.
         * @return The result of the operation.
         */
        void *load(const std::vector<uint8_t> &data, size_t &outSizeBytes) override;
        /**
         * @brief Reloads the requested resource.
         * @param existing The existing used by this operation.
         * @param newData The new data used by this operation.
         * @param[out] outSizeBytes Receives the output size in bytes.
         * @return The result of the operation.
         */
        void *reload(void *existing, const std::vector<uint8_t> &newData,
                     size_t &outSizeBytes) override;
        /**
         * @brief Releases the loaded resource.
         * @param data The data used by this operation.
         */
        void unload(void *data) override;
    };

    // === SceneAsset - Eager geladen, enthält EnTT-Registry Dump ===
    /** @brief Stores the data for the scene asset. */
    struct SceneAsset {
        /** @brief Stores the name. */
        std::string name;
        /** @brief Stores the registry dump. */
        std::vector<uint8_t> registryDump; // Serialisierter EnTT-Registry

        /**
         * @brief Serializes the object.
         * @tparam S Template type used by this operation.
         * @param s The s used by this operation.
         */
        template <typename S> void serialize(S &s) {
            s.value("name", name);
            uint32_t dumpSize = registryDump.size();
            s.value("registryDumpSize", dumpSize);
            if (s.isReading()) {
                registryDump.resize(dumpSize);
            }
        }
    };

    /**
     * @brief Provides the scene loader functionality.
     * @tparam SceneLoader Template type used by this type.
     */
    class SceneLoader : public IAssetLoader {
      public:
        /**
         * @brief Loads the requested resource.
         * @param data The data used by this operation.
         * @param[out] outSizeBytes Receives the output size in bytes.
         * @return The result of the operation.
         */
        void *load(const std::vector<uint8_t> &data, size_t &outSizeBytes) override;
        /**
         * @brief Reloads the requested resource.
         * @param existing The existing used by this operation.
         * @param newData The new data used by this operation.
         * @param[out] outSizeBytes Receives the output size in bytes.
         * @return The result of the operation.
         */
        void *reload(void *existing, const std::vector<uint8_t> &newData,
                     size_t &outSizeBytes) override;
        /**
         * @brief Releases the loaded resource.
         * @param data The data used by this operation.
         */
        void unload(void *data) override;
    };

    // === PrefabAsset - Eager geladen, Single-Entity-Vorlage ===
    /** @brief Stores the data for the prefab asset. */
    struct PrefabAsset {
        /** @brief Stores the name. */
        std::string name;
        /** @brief Stores the entity dump. */
        std::vector<uint8_t> entityDump; // Serialisierte Entity + Komponenten
        /** @brief Stores the component dependencies. */
        std::vector<AssetDependency> componentDependencies;

        /**
         * @brief Serializes the object.
         * @tparam S Template type used by this operation.
         * @param s The s used by this operation.
         */
        template <typename S> void serialize(S &s) {
            s.value("name", name);
            uint32_t dumpSize = entityDump.size();
            s.value("entityDumpSize", dumpSize);
            if (s.isReading()) {
                entityDump.resize(dumpSize);
            }
        }
    };

    /**
     * @brief Provides the prefab loader functionality.
     * @tparam PrefabLoader Template type used by this type.
     */
    class PrefabLoader : public IAssetLoader {
      public:
        /**
         * @brief Loads the requested resource.
         * @param data The data used by this operation.
         * @param[out] outSizeBytes Receives the output size in bytes.
         * @return The result of the operation.
         */
        void *load(const std::vector<uint8_t> &data, size_t &outSizeBytes) override;
        /**
         * @brief Reloads the requested resource.
         * @param existing The existing used by this operation.
         * @param newData The new data used by this operation.
         * @param[out] outSizeBytes Receives the output size in bytes.
         * @return The result of the operation.
         */
        void *reload(void *existing, const std::vector<uint8_t> &newData,
                     size_t &outSizeBytes) override;
        /**
         * @brief Releases the loaded resource.
         * @param data The data used by this operation.
         */
        void unload(void *data) override;
    };

    // === SoundEffectAsset - Eager geladen ===
    /** @brief Stores the data for the sound effect asset. */
    struct SoundEffectAsset {
        /** @brief Stores the sample rate. */
        uint32_t sampleRate = 44100;
        /** @brief Stores the channels. */
        uint32_t channels = 2;
        /** @brief Stores the samples. */
        std::vector<int16_t> samples; // PCM-Daten

        /**
         * @brief Serializes the object.
         * @tparam S Template type used by this operation.
         * @param s The s used by this operation.
         */
        template <typename S> void serialize(S &s) {
            s.value("sampleRate", sampleRate);
            s.value("channels", channels);
            uint32_t sampleCount = samples.size();
            s.value("sampleCount", sampleCount);
            if (s.isReading()) {
                samples.resize(sampleCount);
            }
        }
    };

    /**
     * @brief Provides the sound effect loader functionality.
     * @tparam SoundEffectLoader Template type used by this type.
     */
    class SoundEffectLoader : public IAssetLoader {
      public:
        /**
         * @brief Loads the requested resource.
         * @param data The data used by this operation.
         * @param[out] outSizeBytes Receives the output size in bytes.
         * @return The result of the operation.
         */
        void *load(const std::vector<uint8_t> &data, size_t &outSizeBytes) override;
        /**
         * @brief Reloads the requested resource.
         * @param existing The existing used by this operation.
         * @param newData The new data used by this operation.
         * @param[out] outSizeBytes Receives the output size in bytes.
         * @return The result of the operation.
         */
        void *reload(void *existing, const std::vector<uint8_t> &newData,
                     size_t &outSizeBytes) override;
        /**
         * @brief Releases the loaded resource.
         * @param data The data used by this operation.
         */
        void unload(void *data) override;
    };

} // namespace axiom
