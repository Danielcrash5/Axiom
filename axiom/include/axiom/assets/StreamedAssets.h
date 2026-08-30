#pragma once
#include <atomic>
#include <cstddef>
#include <memory>
#include <vector>

namespace axiom {

    enum class StreamState { Opening = 0, Buffering = 1, Ready = 2, Exhausted = 3, Error = 4 };

    // Interface für gestreamte Assets (Audio, Voxel-Chunks, etc.)
    // Im Gegensatz zu Eager-Assets wird hier nur ein Stream geöffnet,
    // nicht die kompletten Daten geladen.
    class IStreamedAsset {
      public:
        virtual ~IStreamedAsset() = default;

        // Öffne einen Stream auf die Asset-Datei
        virtual bool OpenStream(const std::string &physicalPath) = 0;

        // Fülle einen Buffer mit Daten aus dem Stream
        // Gibt Anzahl der tatsächlich gelesenen Bytes zurück
        virtual size_t FillBuffer(uint8_t *dst, size_t bytesRequested) = 0;

        // Schließe den Stream
        virtual void CloseStream() = 0;

        // Aktueller Zustand des Streams
        virtual StreamState GetState() const = 0;

        // Fortschritt in Prozent (optional)
        virtual float GetProgress() const { return 0.0f; }

        // Gesamtgröße (optional, kann -1 sein wenn unbekannt)
        virtual size_t GetTotalSize() const { return 0; }

        // Seek zu Position (optional)
        virtual bool Seek(size_t position) { return false; }
    };

    // === MusicAsset - Streamed Audio ===
    // Wird nie komplett im RAM gehalten, nur Ring-Buffer während Playback
    class MusicAsset : public IStreamedAsset {
      public:
        static constexpr size_t BUFFER_SIZE = 262144; // 256KB Ring-Buffer

        MusicAsset();
        ~MusicAsset() override;

        bool OpenStream(const std::string &physicalPath) override;
        size_t FillBuffer(uint8_t *dst, size_t bytesRequested) override;
        void CloseStream() override;
        StreamState GetState() const override;
        float GetProgress() const override;
        size_t GetTotalSize() const override;
        bool Seek(size_t position) override;

        uint32_t GetSampleRate() const { return m_SampleRate; }
        uint32_t GetChannels() const { return m_Channels; }
        size_t GetBytePosition() const { return m_BytePosition; }

      private:
        StreamState m_State = StreamState::Opening;
        uint32_t m_SampleRate = 44100;
        uint32_t m_Channels = 2;
        size_t m_TotalBytes = 0;
        size_t m_BytePosition = 0;
        void *m_FileHandle = nullptr;
        void *m_DecoderContext = nullptr; // z.B. ffmpeg/libsndfile context
    };

    // Loader für MusicAsset
    class MusicLoader : public IAssetLoader {
      public:
        void *Load(const std::vector<uint8_t> &data, size_t &outSizeBytes) override;
        void Unload(void *data) override;
    };

} // namespace axiom
