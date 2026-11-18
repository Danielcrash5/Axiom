/**
 * @file axiom/src/assets/AssetTypes.cpp
 * @brief Implements the asset types module.
 */

#include <axiom/assets/AssetTypes.h>
#include <cstring>

namespace axiom {

    // === TextureLoader ===

    void *TextureLoader::load(const std::vector<uint8_t> &data, size_t &outSizeBytes) {
        if (data.size() < sizeof(uint32_t) * 3) {
            return nullptr;
        }

        auto *texture = new TextureAsset();
        BinarySerializer ser;
        ser.loadFromBytes(data);

        // Lese Metadaten
        ser.value("width", texture->width);
        ser.value("height", texture->height);
        ser.value("channels", texture->channels);

        uint32_t dataSize = 0;
        ser.value("dataSize", dataSize);

        texture->pixelData.resize(dataSize);
        // TODO: Lese Pixel-Daten

        outSizeBytes = sizeof(TextureAsset) + texture->pixelData.size();
        return texture;
    }

    void *TextureLoader::reload(void *existing, const std::vector<uint8_t> &newData,
                                 size_t &outSizeBytes) {
        void *replacement = load(newData, outSizeBytes);
        if (replacement && existing) unload(existing);
        return replacement;
    }

    void TextureLoader::unload(void *data) { delete static_cast<TextureAsset *>(data); }

    // === MeshLoader ===

    void *MeshLoader::load(const std::vector<uint8_t> &data, size_t &outSizeBytes) {
        auto *mesh = new MeshAsset();
        BinarySerializer ser;
        ser.loadFromBytes(data);

        uint32_t vertexCount = 0;
        ser.value("vertexCount", vertexCount);

        uint32_t indexCount = 0;
        ser.value("indexCount", indexCount);

        mesh->vertices.resize(vertexCount);
        mesh->indices.resize(indexCount);
        // TODO: Lese Vertex- und Index-Daten

        outSizeBytes = sizeof(MeshAsset) + vertexCount * sizeof(MeshAsset::Vertex) +
                       indexCount * sizeof(uint32_t);
        return mesh;
    }

    void *MeshLoader::reload(void *existing, const std::vector<uint8_t> &newData,
                              size_t &outSizeBytes) {
        void *replacement = load(newData, outSizeBytes);
        if (replacement && existing) unload(existing);
        return replacement;
    }

    void MeshLoader::unload(void *data) { delete static_cast<MeshAsset *>(data); }

    // === MaterialLoader ===

    void *MaterialLoader::load(const std::vector<uint8_t> &data, size_t &outSizeBytes) {
        auto *material = new MaterialAsset();
        BinarySerializer ser;
        ser.loadFromBytes(data);

        ser.value("name", material->name);
        ser.value("metallic", material->metallic);
        ser.value("roughness", material->roughness);
        // TODO: Lese Farben und Texture-UUIDs

        outSizeBytes = sizeof(MaterialAsset) + material->name.size();
        return material;
    }

    void *MaterialLoader::reload(void *existing, const std::vector<uint8_t> &newData,
                                  size_t &outSizeBytes) {
        void *replacement = load(newData, outSizeBytes);
        if (replacement && existing) unload(existing);
        return replacement;
    }

    void MaterialLoader::unload(void *data) { delete static_cast<MaterialAsset *>(data); }

    // === SceneLoader ===

    void *SceneLoader::load(const std::vector<uint8_t> &data, size_t &outSizeBytes) {
        auto *scene = new SceneAsset();
        BinarySerializer ser;
        ser.loadFromBytes(data);

        ser.value("name", scene->name);

        uint32_t dumpSize = 0;
        ser.value("registryDumpSize", dumpSize);
        scene->registryDump.resize(dumpSize);
        // TODO: Lese Registry-Dump

        outSizeBytes = sizeof(SceneAsset) + scene->registryDump.size();
        return scene;
    }

    void *SceneLoader::reload(void *existing, const std::vector<uint8_t> &newData,
                               size_t &outSizeBytes) {
        void *replacement = load(newData, outSizeBytes);
        if (replacement && existing) unload(existing);
        return replacement;
    }

    void SceneLoader::unload(void *data) { delete static_cast<SceneAsset *>(data); }

    // === PrefabLoader ===

    void *PrefabLoader::load(const std::vector<uint8_t> &data, size_t &outSizeBytes) {
        auto *prefab = new PrefabAsset();
        BinarySerializer ser;
        ser.loadFromBytes(data);

        ser.value("name", prefab->name);

        uint32_t dumpSize = 0;
        ser.value("entityDumpSize", dumpSize);
        prefab->entityDump.resize(dumpSize);
        // TODO: Lese Entity-Dump

        outSizeBytes = sizeof(PrefabAsset) + prefab->entityDump.size();
        return prefab;
    }

    void *PrefabLoader::reload(void *existing, const std::vector<uint8_t> &newData,
                                size_t &outSizeBytes) {
        void *replacement = load(newData, outSizeBytes);
        if (replacement && existing) unload(existing);
        return replacement;
    }

    void PrefabLoader::unload(void *data) { delete static_cast<PrefabAsset *>(data); }

    // === SoundEffectLoader ===

    void *SoundEffectLoader::load(const std::vector<uint8_t> &data, size_t &outSizeBytes) {
        auto *sound = new SoundEffectAsset();
        BinarySerializer ser;
        ser.loadFromBytes(data);

        ser.value("sampleRate", sound->sampleRate);
        ser.value("channels", sound->channels);

        uint32_t sampleCount = 0;
        ser.value("sampleCount", sampleCount);
        sound->samples.resize(sampleCount);
        // TODO: Lese Sample-Daten

        outSizeBytes = sizeof(SoundEffectAsset) + sound->samples.size() * sizeof(int16_t);
        return sound;
    }

    void *SoundEffectLoader::reload(void *existing, const std::vector<uint8_t> &newData,
                                     size_t &outSizeBytes) {
        void *replacement = load(newData, outSizeBytes);
        if (replacement && existing) unload(existing);
        return replacement;
    }

    void SoundEffectLoader::unload(void *data) { delete static_cast<SoundEffectAsset *>(data); }

} // namespace axiom
