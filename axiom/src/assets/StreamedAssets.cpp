#include <axiom/assets/StreamedAssets.h>
#include <axiom/assets/VFS.h>
#include <iostream>

namespace axiom {

    // === MusicAsset ===

    MusicAsset::MusicAsset() {}

    MusicAsset::~MusicAsset() { CloseStream(); }

    bool MusicAsset::OpenStream(const std::string &physicalPath) {
        // TODO: Implementiere Audio-Decoding mit libsndfile oder ffmpeg
        // Für jetzt: Placeholder
        m_State = StreamState::Ready;
        return true;
    }

    size_t MusicAsset::FillBuffer(uint8_t *dst, size_t bytesRequested) {
        if (m_State != StreamState::Ready) {
            return 0;
        }

        // TODO: Lese von Decoder-Kontext in Ring-Buffer
        // Für jetzt: Rückgabe 0
        return 0;
    }

    void MusicAsset::CloseStream() {
        if (m_FileHandle) {
            // TODO: Schließe Datei und Decoder
            m_FileHandle = nullptr;
        }
        m_State = StreamState::Opening;
    }

    StreamState MusicAsset::GetState() const { return m_State; }

    float MusicAsset::GetProgress() const {
        if (m_TotalBytes == 0) {
            return 0.0f;
        }
        return static_cast<float>(m_BytePosition) / static_cast<float>(m_TotalBytes);
    }

    size_t MusicAsset::GetTotalSize() const { return m_TotalBytes; }

    bool MusicAsset::Seek(size_t position) {
        if (position > m_TotalBytes) {
            return false;
        }
        m_BytePosition = position;
        // TODO: Seek in Decoder
        return true;
    }

    // === MusicLoader ===

    void *MusicLoader::Load(const std::vector<uint8_t> &data, size_t &outSizeBytes) {
        auto *music = new MusicAsset();
        // TODO: Initialisiere Decoder mit Metadaten
        outSizeBytes = sizeof(MusicAsset);
        return music;
    }

    void MusicLoader::Unload(void *data) { delete static_cast<MusicAsset *>(data); }

} // namespace axiom
