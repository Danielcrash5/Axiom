#include <axiom/assets/AssetTypes.h>
#include <cstring>

namespace axiom {

    // === TextureLoader ===

    void *TextureLoader::Load(const std::vector<uint8_t> &data, size_t &outSizeBytes) {
        if (data.size() < sizeof(uint32_t) * 3) {
            return nullptr;
        }

        auto *texture = new TextureAsset();
        BinarySerializer ser;
        ser.LoadFromBytes(data);

        // Lese Metadaten
        ser.Value("width", texture->width);
        ser.Value("height", texture->height);
        ser.Value("channels", texture->channels);

        uint32_t dataSize = 0;
        ser.Value("dataSize", dataSize);

        texture->pixelData.resize(dataSize);
        // TODO: Lese Pixel-Daten

        outSizeBytes = sizeof(TextureAsset) + texture->pixelData.size();
        return texture;
    }

    void *TextureLoader::Reload(void *existing, const std::vector<uint8_t> &newData,
                                 size_t &outSizeBytes) {
        delete static_cast<TextureAsset *>(existing);
        return Load(newData, outSizeBytes);
    }

    void TextureLoader::Unload(void *data) { delete static_cast<TextureAsset *>(data); }

    // === MeshLoader ===

    void *MeshLoader::Load(const std::vector<uint8_t> &data, size_t &outSizeBytes) {
        auto *mesh = new MeshAsset();
        BinarySerializer ser;
        ser.LoadFromBytes(data);

        uint32_t vertexCount = 0;
        ser.Value("vertexCount", vertexCount);

        uint32_t indexCount = 0;
        ser.Value("indexCount", indexCount);

        mesh->vertices.resize(vertexCount);
        mesh->indices.resize(indexCount);
        // TODO: Lese Vertex- und Index-Daten

        outSizeBytes = sizeof(MeshAsset) + vertexCount * sizeof(MeshAsset::Vertex) +
                       indexCount * sizeof(uint32_t);
        return mesh;
    }

    void *MeshLoader::Reload(void *existing, const std::vector<uint8_t> &newData,
                              size_t &outSizeBytes) {
        delete static_cast<MeshAsset *>(existing);
        return Load(newData, outSizeBytes);
    }

    void MeshLoader::Unload(void *data) { delete static_cast<MeshAsset *>(data); }

    // === MaterialLoader ===

    void *MaterialLoader::Load(const std::vector<uint8_t> &data, size_t &outSizeBytes) {
        auto *material = new MaterialAsset();
        BinarySerializer ser;
        ser.LoadFromBytes(data);

        ser.Value("name", material->name);
        ser.Value("metallic", material->metallic);
        ser.Value("roughness", material->roughness);
        // TODO: Lese Farben und Texture-UUIDs

        outSizeBytes = sizeof(MaterialAsset) + material->name.size();
        return material;
    }

    void *MaterialLoader::Reload(void *existing, const std::vector<uint8_t> &newData,
                                  size_t &outSizeBytes) {
        delete static_cast<MaterialAsset *>(existing);
        return Load(newData, outSizeBytes);
    }

    void MaterialLoader::Unload(void *data) { delete static_cast<MaterialAsset *>(data); }

    // === SceneLoader ===

    void *SceneLoader::Load(const std::vector<uint8_t> &data, size_t &outSizeBytes) {
        auto *scene = new SceneAsset();
        BinarySerializer ser;
        ser.LoadFromBytes(data);

        ser.Value("name", scene->name);

        uint32_t dumpSize = 0;
        ser.Value("registryDumpSize", dumpSize);
        scene->registryDump.resize(dumpSize);
        // TODO: Lese Registry-Dump

        outSizeBytes = sizeof(SceneAsset) + scene->registryDump.size();
        return scene;
    }

    void *SceneLoader::Reload(void *existing, const std::vector<uint8_t> &newData,
                               size_t &outSizeBytes) {
        delete static_cast<SceneAsset *>(existing);
        return Load(newData, outSizeBytes);
    }

    void SceneLoader::Unload(void *data) { delete static_cast<SceneAsset *>(data); }

    // === PrefabLoader ===

    void *PrefabLoader::Load(const std::vector<uint8_t> &data, size_t &outSizeBytes) {
        auto *prefab = new PrefabAsset();
        BinarySerializer ser;
        ser.LoadFromBytes(data);

        ser.Value("name", prefab->name);

        uint32_t dumpSize = 0;
        ser.Value("entityDumpSize", dumpSize);
        prefab->entityDump.resize(dumpSize);
        // TODO: Lese Entity-Dump

        outSizeBytes = sizeof(PrefabAsset) + prefab->entityDump.size();
        return prefab;
    }

    void *PrefabLoader::Reload(void *existing, const std::vector<uint8_t> &newData,
                                size_t &outSizeBytes) {
        delete static_cast<PrefabAsset *>(existing);
        return Load(newData, outSizeBytes);
    }

    void PrefabLoader::Unload(void *data) { delete static_cast<PrefabAsset *>(data); }

    // === SoundEffectLoader ===

    void *SoundEffectLoader::Load(const std::vector<uint8_t> &data, size_t &outSizeBytes) {
        auto *sound = new SoundEffectAsset();
        BinarySerializer ser;
        ser.LoadFromBytes(data);

        ser.Value("sampleRate", sound->sampleRate);
        ser.Value("channels", sound->channels);

        uint32_t sampleCount = 0;
        ser.Value("sampleCount", sampleCount);
        sound->samples.resize(sampleCount);
        // TODO: Lese Sample-Daten

        outSizeBytes = sizeof(SoundEffectAsset) + sound->samples.size() * sizeof(int16_t);
        return sound;
    }

    void *SoundEffectLoader::Reload(void *existing, const std::vector<uint8_t> &newData,
                                     size_t &outSizeBytes) {
        delete static_cast<SoundEffectAsset *>(existing);
        return Load(newData, outSizeBytes);
    }

    void SoundEffectLoader::Unload(void *data) { delete static_cast<SoundEffectAsset *>(data); }

} // namespace axiom
