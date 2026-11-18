/**
 * @file axiom/include/axiom/assets/StreamedAssets.h
 * @brief Declares the public interface for the streamed assets.
 */

#pragma once
#include <atomic>
#include <cstddef>
#include <memory>
#include <vector>
#include "AssetManager.h"

namespace axiom {

    /**
     * @brief Defines the available stream state values.
     * @tparam StreamState Template type used by this type.
     */
    enum class StreamState { opening = 0, buffering = 1, ready = 2, exhausted = 3, error = 4 };

    // Interface für gestreamte Assets (Audio, Voxel-Chunks, etc.)
    // Im Gegensatz zu Eager-Assets wird hier nur ein Stream geöffnet,
    // nicht die kompletten Daten geladen.
    /**
     * @brief Defines the interface for the streamed asset.
     * @tparam IStreamedAsset Template type used by this type.
     */
    class IStreamedAsset {
      public:
        /**
         * @brief Releases resources owned by IStreamedAsset.
         */
        virtual ~IStreamedAsset() = default;

        // Öffne einen Stream auf die Asset-Datei
        /**
         * @brief Opens stream.
         * @param physicalPath The physical path used by this operation.
         * @return The result of the operation.
         */
        virtual bool openStream(const std::string &physicalPath) = 0;

        // Fülle einen Buffer mit Daten aus dem Stream
        // Gibt Anzahl der tatsächlich gelesenen Bytes zurück
        /**
         * @brief Fills the buffer with the requested value.
         * @param dst The dst used by this operation.
         * @param bytesRequested The bytes requested used by this operation.
         * @return The result of the operation.
         */
        virtual size_t fillBuffer(uint8_t *dst, size_t bytesRequested) = 0;

        // Schließe den Stream
        /**
         * @brief Closes stream.
         */
        virtual void closeStream() = 0;

        // Aktueller Zustand des Streams
        /**
         * @brief Returns the state.
         * @return The result of the operation.
         */
        virtual StreamState getState() const = 0;

        // Fortschritt in Prozent (optional)
        /**
         * @brief Returns the progress.
         * @return The result of the operation.
         */
        virtual float getProgress() const { return 0.0f; }

        // Gesamtgröße (optional, kann -1 sein wenn unbekannt)
        /**
         * @brief Returns the total size.
         * @return The result of the operation.
         */
        virtual size_t getTotalSize() const { return 0; }

        // Seek zu Position (optional)
        /**
         * @brief Moves the stream cursor to the requested position.
         * @param position The position used by this operation.
         * @return The result of the operation.
         */
        virtual bool seek(size_t position) { return false; }
    };

    // === MusicAsset - Streamed Audio ===
    // Wird nie komplett im RAM gehalten, nur Ring-Buffer während Playback
    /**
     * @brief Provides the music asset functionality.
     * @tparam MusicAsset Template type used by this type.
     */
    class MusicAsset : public IStreamedAsset {
      public:
        /** @brief Stores the buffer size. */
        static constexpr size_t bufferSize = 262144; // 256KB Ring-Buffer

        /**
         * @brief Creates an instance of music asset.
         */
        MusicAsset();
        /**
         * @brief Releases resources owned by MusicAsset.
         */
        ~MusicAsset() override;

        /**
         * @brief Opens stream.
         * @param physicalPath The physical path used by this operation.
         * @return The result of the operation.
         */
        bool openStream(const std::string &physicalPath) override;
        /**
         * @brief Fills the buffer with the requested value.
         * @param dst The dst used by this operation.
         * @param bytesRequested The bytes requested used by this operation.
         * @return The result of the operation.
         */
        size_t fillBuffer(uint8_t *dst, size_t bytesRequested) override;
        /**
         * @brief Closes stream.
         */
        void closeStream() override;
        /**
         * @brief Returns the state.
         * @return The result of the operation.
         */
        StreamState getState() const override;
        /**
         * @brief Returns the progress.
         * @return The result of the operation.
         */
        float getProgress() const override;
        /**
         * @brief Returns the total size.
         * @return The result of the operation.
         */
        size_t getTotalSize() const override;
        /**
         * @brief Moves the stream cursor to the requested position.
         * @param position The position used by this operation.
         * @return The result of the operation.
         */
        bool seek(size_t position) override;

        /**
         * @brief Returns the sample rate.
         * @return The result of the operation.
         */
        uint32_t getSampleRate() const { return m_sampleRate; }
        /**
         * @brief Returns the channels.
         * @return The result of the operation.
         */
        uint32_t getChannels() const { return m_channels; }
        /**
         * @brief Returns the byte position.
         * @return The result of the operation.
         */
        size_t getBytePosition() const { return m_bytePosition; }

      private:
        /** @brief Stores the state. */
        StreamState m_state = StreamState::opening;
        /** @brief Stores the sample rate. */
        uint32_t m_sampleRate = 44100;
        /** @brief Stores the channels. */
        uint32_t m_channels = 2;
        /** @brief Stores the total bytes. */
        size_t m_totalBytes = 0;
        /** @brief Stores the byte position. */
        size_t m_bytePosition = 0;
        /** @brief Stores the file handle. */
        void *m_fileHandle = nullptr;
        /** @brief Stores the decoder context. */
        void *m_decoderContext = nullptr; // z.B. ffmpeg/libsndfile context
    };

    // Loader für MusicAsset
    /**
     * @brief Provides the music loader functionality.
     * @tparam MusicLoader Template type used by this type.
     */
    class MusicLoader : public IAssetLoader {
      public:
        /**
         * @brief Loads the requested resource.
         * @param data The data used by this operation.
         * @param[out] outSizeBytes Receives the output size in bytes.
         * @return The result of the operation.
         */
        void *load(const std::vector<uint8_t> &data, size_t &outSizeBytes) override;
        /**
         * @brief Releases the loaded resource.
         * @param data The data used by this operation.
         */
        void unload(void *data) override;
    };

} // namespace axiom
