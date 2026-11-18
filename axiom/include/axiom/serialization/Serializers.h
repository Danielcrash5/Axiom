/**
 * @file axiom/include/axiom/serialization/Serializers.h
 * @brief Declares the public interface for the serializers.
 */

#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <axiom/assets/TypedUUID.h>

namespace axiom {

    /**
     * @brief Defines the available serializer mode values.
     * @tparam SerializerMode Template type used by this type.
     */
    enum class SerializerMode { reading = 0, writing = 1 };

    // JSON-Serialisierung (für .axmeta, editierbare Dateien)
    /**
     * @brief Provides the JSON serializer functionality.
     * @tparam JsonSerializer Template type used by this type.
     */
    class JsonSerializer {
      public:
        /**
         * @brief Provides the mode type alias.
         */
        using Mode = SerializerMode;
        /**
         * @brief Creates an instance of JSON serializer.
         */
        JsonSerializer();
        /**
         * @brief Releases resources owned by JsonSerializer.
         */
        ~JsonSerializer();

        // Laden aus JSON-Text
        /**
         * @brief Loads from string.
         * @param jsonText The JSON text used by this operation.
         * @return The result of the operation.
         */
        bool loadFromString(const std::string &jsonText);

        // Speichern als JSON-Text
        /**
         * @brief Saves to string.
         * @param pretty The pretty used by this operation.
         * @return The result of the operation.
         */
        std::string saveToString(bool pretty = true) const;

        /**
         * @brief Returns the mode.
         * @return The result of the operation.
         */
        Mode getMode() const;
        /**
         * @brief Reports whether reading.
         * @return The result of the operation.
         */
        bool isReading() const { return getMode() == Mode::reading; }
        /**
         * @brief Reports whether writing.
         * @return The result of the operation.
         */
        bool isWriting() const { return getMode() == Mode::writing; }

        /**
         * @brief Reads or writes a value during serialization.
         * @param key The key used by this operation.
         * @param value The value used by this operation.
         * @return The result of the operation.
         */
        bool value(const char *key, bool &value);
        /**
         * @brief Reads or writes a value during serialization.
         * @param key The key used by this operation.
         * @param value The value used by this operation.
         * @return The result of the operation.
         */
        bool value(const char *key, int8_t &value);
        /**
         * @brief Reads or writes a value during serialization.
         * @param key The key used by this operation.
         * @param value The value used by this operation.
         * @return The result of the operation.
         */
        bool value(const char *key, uint8_t &value);
        /**
         * @brief Reads or writes a value during serialization.
         * @param key The key used by this operation.
         * @param value The value used by this operation.
         * @return The result of the operation.
         */
        bool value(const char *key, int16_t &value);
        /**
         * @brief Reads or writes a value during serialization.
         * @param key The key used by this operation.
         * @param value The value used by this operation.
         * @return The result of the operation.
         */
        bool value(const char *key, uint16_t &value);
        /**
         * @brief Reads or writes a value during serialization.
         * @param key The key used by this operation.
         * @param value The value used by this operation.
         * @return The result of the operation.
         */
        bool value(const char *key, int32_t &value);
        /**
         * @brief Reads or writes a value during serialization.
         * @param key The key used by this operation.
         * @param value The value used by this operation.
         * @return The result of the operation.
         */
        bool value(const char *key, uint32_t &value);
        /**
         * @brief Reads or writes a value during serialization.
         * @param key The key used by this operation.
         * @param value The value used by this operation.
         * @return The result of the operation.
         */
        bool value(const char *key, int64_t &value);
        /**
         * @brief Reads or writes a value during serialization.
         * @param key The key used by this operation.
         * @param value The value used by this operation.
         * @return The result of the operation.
         */
        bool value(const char *key, uint64_t &value);
        /**
         * @brief Reads or writes a value during serialization.
         * @param key The key used by this operation.
         * @param value The value used by this operation.
         * @return The result of the operation.
         */
        bool value(const char *key, float &value);
        /**
         * @brief Reads or writes a value during serialization.
         * @param key The key used by this operation.
         * @param value The value used by this operation.
         * @return The result of the operation.
         */
        bool value(const char *key, double &value);
        /**
         * @brief Reads or writes a value during serialization.
         * @param key The key used by this operation.
         * @param value The value used by this operation.
         * @return The result of the operation.
         */
        bool value(const char *key, std::string &value);

        /**
         * @brief Reads or writes a value during serialization.
         * @tparam T Template type used by this operation.
         */
        template <typename T> void value(const char *, T &) {
          static_assert(sizeof(T) == 0, "Serializer value type is not supported");
        }

        /**
         * @brief Begins array.
         * @param key The key used by this operation.
         * @param count The count used by this operation.
         * @return The result of the operation.
         */
        bool beginArray(const char *key, uint32_t &count);
        /**
         * @brief Ends array.
         * @return The result of the operation.
         */
        bool endArray();
        /**
         * @brief Begins object.
         * @param key The key used by this operation.
         * @return The result of the operation.
         */
        bool beginObject(const char *key);
        /**
         * @brief Ends object.
         * @return The result of the operation.
         */
        bool endObject();
        /**
         * @brief Reports whether key.
         * @param key The key used by this operation.
         * @return The result of the operation.
         */
        bool hasKey(const char *key) const;
        /**
         * @brief Returns the debug info.
         * @return The result of the operation.
         */
        std::string getDebugInfo() const;

      private:
        /** @brief Stores the JSON. */
        void *m_json = nullptr; // nlohmann::json*
        /** @brief Stores the mode. */
        Mode m_mode = Mode::writing;
    };

    // Binary-Serialisierung (für .axasset, nicht editierbar)
    /**
     * @brief Provides the binary serializer functionality.
     * @tparam BinarySerializer Template type used by this type.
     */
    class BinarySerializer {
      public:
        /**
         * @brief Provides the mode type alias.
         */
        using Mode = SerializerMode;
        /**
         * @brief Creates an instance of binary serializer.
         */
        BinarySerializer();
        /**
         * @brief Releases resources owned by BinarySerializer.
         */
        ~BinarySerializer();

        // Laden aus Binary-Daten
        /**
         * @brief Loads from bytes.
         * @param data The data used by this operation.
         * @return The result of the operation.
         */
        bool loadFromBytes(const std::vector<uint8_t> &data);

        // Speichern als Binary-Daten
        /**
         * @brief Saves to bytes.
         * @return The result of the operation.
         */
        std::vector<uint8_t> saveToBytes() const;

        /**
         * @brief Returns the mode.
         * @return The result of the operation.
         */
        Mode getMode() const;
        /**
         * @brief Reports whether reading.
         * @return The result of the operation.
         */
        bool isReading() const { return getMode() == Mode::reading; }
        /**
         * @brief Reports whether writing.
         * @return The result of the operation.
         */
        bool isWriting() const { return getMode() == Mode::writing; }

        /**
         * @brief Reads or writes a value during serialization.
         * @param key The key used by this operation.
         * @param value The value used by this operation.
         * @return The result of the operation.
         */
        bool value(const char *key, bool &value);
        /**
         * @brief Reads or writes a value during serialization.
         * @param key The key used by this operation.
         * @param value The value used by this operation.
         * @return The result of the operation.
         */
        bool value(const char *key, int8_t &value);
        /**
         * @brief Reads or writes a value during serialization.
         * @param key The key used by this operation.
         * @param value The value used by this operation.
         * @return The result of the operation.
         */
        bool value(const char *key, uint8_t &value);
        /**
         * @brief Reads or writes a value during serialization.
         * @param key The key used by this operation.
         * @param value The value used by this operation.
         * @return The result of the operation.
         */
        bool value(const char *key, int16_t &value);
        /**
         * @brief Reads or writes a value during serialization.
         * @param key The key used by this operation.
         * @param value The value used by this operation.
         * @return The result of the operation.
         */
        bool value(const char *key, uint16_t &value);
        /**
         * @brief Reads or writes a value during serialization.
         * @param key The key used by this operation.
         * @param value The value used by this operation.
         * @return The result of the operation.
         */
        bool value(const char *key, int32_t &value);
        /**
         * @brief Reads or writes a value during serialization.
         * @param key The key used by this operation.
         * @param value The value used by this operation.
         * @return The result of the operation.
         */
        bool value(const char *key, uint32_t &value);
        /**
         * @brief Reads or writes a value during serialization.
         * @param key The key used by this operation.
         * @param value The value used by this operation.
         * @return The result of the operation.
         */
        bool value(const char *key, int64_t &value);
        /**
         * @brief Reads or writes a value during serialization.
         * @param key The key used by this operation.
         * @param value The value used by this operation.
         * @return The result of the operation.
         */
        bool value(const char *key, uint64_t &value);
        /**
         * @brief Reads or writes a value during serialization.
         * @param key The key used by this operation.
         * @param value The value used by this operation.
         * @return The result of the operation.
         */
        bool value(const char *key, float &value);
        /**
         * @brief Reads or writes a value during serialization.
         * @param key The key used by this operation.
         * @param value The value used by this operation.
         * @return The result of the operation.
         */
        bool value(const char *key, double &value);
        /**
         * @brief Reads or writes a value during serialization.
         * @param key The key used by this operation.
         * @param value The value used by this operation.
         * @return The result of the operation.
         */
        bool value(const char *key, std::string &value);

        /**
         * @brief Reads or writes a value during serialization.
         * @tparam T Template type used by this operation.
         */
        template <typename T> void value(const char *, T &) {
          static_assert(sizeof(T) == 0, "Serializer value type is not supported");
        }

        /**
         * @brief Begins array.
         * @param key The key used by this operation.
         * @param count The count used by this operation.
         * @return The result of the operation.
         */
        bool beginArray(const char *key, uint32_t &count);
        /**
         * @brief Ends array.
         * @return The result of the operation.
         */
        bool endArray();
        /**
         * @brief Begins object.
         * @param key The key used by this operation.
         * @return The result of the operation.
         */
        bool beginObject(const char *key);
        /**
         * @brief Ends object.
         * @return The result of the operation.
         */
        bool endObject();
        /**
         * @brief Reports whether key.
         * @param key The key used by this operation.
         * @return The result of the operation.
         */
        bool hasKey(const char *key) const;
        /**
         * @brief Returns the debug info.
         * @return The result of the operation.
         */
        std::string getDebugInfo() const;

      private:
        /** @brief Stores the buffer. */
        std::vector<uint8_t> m_buffer;
        /** @brief Stores the position. */
        size_t m_position = 0;
        /** @brief Stores the mode. */
        Mode m_mode = Mode::writing;

        /**
         * @brief Writes bytes.
         * @param data The data used by this operation.
         * @param size The size used by this operation.
         */
        void writeBytes(const uint8_t *data, size_t size);
        /**
         * @brief Reads bytes.
         * @param data The data used by this operation.
         * @param size The size used by this operation.
         * @return The result of the operation.
         */
        bool readBytes(uint8_t *data, size_t size);
    };

    template <> inline void JsonSerializer::value<glm::vec4>(const char *key,
                                                               glm::vec4 &serializedValue) {
        std::string prefix(key);
        value((prefix + ".x").c_str(), serializedValue.x);
        value((prefix + ".y").c_str(), serializedValue.y);
        value((prefix + ".z").c_str(), serializedValue.z);
        value((prefix + ".w").c_str(), serializedValue.w);
    }

    template <> inline void BinarySerializer::value<glm::vec4>(const char *,
                                                                 glm::vec4 &serializedValue) {
        value("x", serializedValue.x);
        value("y", serializedValue.y);
        value("z", serializedValue.z);
        value("w", serializedValue.w);
    }

    template <> inline void JsonSerializer::value<TypedUUID>(const char *key,
                                                               TypedUUID &serializedValue) {
        std::string serialized = serializedValue.toString();
        value(key, serialized);
        if (isReading()) serializedValue = TypedUUID::fromString(serialized);
    }

    template <> inline void BinarySerializer::value<TypedUUID>(const char *key,
                                                                 TypedUUID &serializedValue) {
        std::string serialized = serializedValue.toString();
        value(key, serialized);
        if (isReading()) serializedValue = TypedUUID::fromString(serialized);
    }

    } // namespace axiom