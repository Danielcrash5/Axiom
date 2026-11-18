/**
 * @file axiom/src/assets/StreamedAssets.cpp
 * @brief Implements the streamed assets module.
 */

#include <axiom/assets/StreamedAssets.h>
#include <axiom/assets/VFS.h>
#include <iostream>

namespace axiom {

    // === MusicAsset ===

    MusicAsset::MusicAsset() {}

    MusicAsset::~MusicAsset() { closeStream(); }

    bool MusicAsset::openStream(const std::string &physicalPath) {
        // TODO: Implementiere Audio-Decoding mit libsndfile oder ffmpeg
        // Für jetzt: Placeholder
        m_state = StreamState::ready;
        return true;
    }

    size_t MusicAsset::fillBuffer(uint8_t *dst, size_t bytesRequested) {
        if (m_state != StreamState::ready) {
            return 0;
        }

        // TODO: Lese von Decoder-Kontext in Ring-Buffer
        // Für jetzt: Rückgabe 0
        return 0;
    }

    void MusicAsset::closeStream() {
        if (m_fileHandle) {
            // TODO: Schließe Datei und Decoder
            m_fileHandle = nullptr;
        }
        m_state = StreamState::opening;
    }

    StreamState MusicAsset::getState() const { return m_state; }

    float MusicAsset::getProgress() const {
        if (m_totalBytes == 0) {
            return 0.0f;
        }
        return static_cast<float>(m_bytePosition) / static_cast<float>(m_totalBytes);
    }

    size_t MusicAsset::getTotalSize() const { return m_totalBytes; }

    bool MusicAsset::seek(size_t position) {
        if (position > m_totalBytes) {
            return false;
        }
        m_bytePosition = position;
        // TODO: Seek in Decoder
        return true;
    }

    // === MusicLoader ===

    void *MusicLoader::load(const std::vector<uint8_t> &data, size_t &outSizeBytes) {
        auto *music = new MusicAsset();
        // TODO: Initialisiere Decoder mit Metadaten
        outSizeBytes = sizeof(MusicAsset);
        return music;
    }

    void MusicLoader::unload(void *data) { delete static_cast<MusicAsset *>(data); }

} // namespace axiom
