#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <axiom/assets/TypedUUID.h>

namespace axiom {

    enum class SerializerMode { Reading = 0, Writing = 1 };

    // JSON-Serialisierung (für .axmeta, editierbare Dateien)
    class JsonSerializer {
      public:
        using Mode = SerializerMode;
        JsonSerializer();
        ~JsonSerializer();

        // Laden aus JSON-Text
        bool LoadFromString(const std::string &jsonText);

        // Speichern als JSON-Text
        std::string SaveToString(bool pretty = true) const;

        Mode GetMode() const;
        bool IsReading() const { return GetMode() == Mode::Reading; }
        bool IsWriting() const { return GetMode() == Mode::Writing; }

        bool Value(const char *key, bool &value);
        bool Value(const char *key, int8_t &value);
        bool Value(const char *key, uint8_t &value);
        bool Value(const char *key, int16_t &value);
        bool Value(const char *key, uint16_t &value);
        bool Value(const char *key, int32_t &value);
        bool Value(const char *key, uint32_t &value);
        bool Value(const char *key, int64_t &value);
        bool Value(const char *key, uint64_t &value);
        bool Value(const char *key, float &value);
        bool Value(const char *key, double &value);
        bool Value(const char *key, std::string &value);

        template <typename T> void Value(const char *, T &) {
          static_assert(sizeof(T) == 0, "Serializer value type is not supported");
        }

        bool BeginArray(const char *key, uint32_t &count);
        bool EndArray();
        bool BeginObject(const char *key);
        bool EndObject();
        bool HasKey(const char *key) const;
        std::string GetDebugInfo() const;

      private:
        void *m_Json = nullptr; // nlohmann::json*
        Mode m_Mode = Mode::Writing;
    };

    // Binary-Serialisierung (für .axasset, nicht editierbar)
    class BinarySerializer {
      public:
        using Mode = SerializerMode;
        BinarySerializer();
        ~BinarySerializer();

        // Laden aus Binary-Daten
        bool LoadFromBytes(const std::vector<uint8_t> &data);

        // Speichern als Binary-Daten
        std::vector<uint8_t> SaveToBytes() const;

        Mode GetMode() const;
        bool IsReading() const { return GetMode() == Mode::Reading; }
        bool IsWriting() const { return GetMode() == Mode::Writing; }

        bool Value(const char *key, bool &value);
        bool Value(const char *key, int8_t &value);
        bool Value(const char *key, uint8_t &value);
        bool Value(const char *key, int16_t &value);
        bool Value(const char *key, uint16_t &value);
        bool Value(const char *key, int32_t &value);
        bool Value(const char *key, uint32_t &value);
        bool Value(const char *key, int64_t &value);
        bool Value(const char *key, uint64_t &value);
        bool Value(const char *key, float &value);
        bool Value(const char *key, double &value);
        bool Value(const char *key, std::string &value);

        template <typename T> void Value(const char *, T &) {
          static_assert(sizeof(T) == 0, "Serializer value type is not supported");
        }

        bool BeginArray(const char *key, uint32_t &count);
        bool EndArray();
        bool BeginObject(const char *key);
        bool EndObject();
        bool HasKey(const char *key) const;
        std::string GetDebugInfo() const;

      private:
        std::vector<uint8_t> m_Buffer;
        size_t m_Position = 0;
        Mode m_Mode = Mode::Writing;

        void WriteBytes(const uint8_t *data, size_t size);
        bool ReadBytes(uint8_t *data, size_t size);
    };

    template <> inline void JsonSerializer::Value<glm::vec4>(const char *key,
                                                               glm::vec4 &value) {
        std::string prefix(key);
        Value((prefix + ".x").c_str(), value.x);
        Value((prefix + ".y").c_str(), value.y);
        Value((prefix + ".z").c_str(), value.z);
        Value((prefix + ".w").c_str(), value.w);
    }

    template <> inline void BinarySerializer::Value<glm::vec4>(const char *,
                                                                 glm::vec4 &value) {
        Value("x", value.x);
        Value("y", value.y);
        Value("z", value.z);
        Value("w", value.w);
    }

    template <> inline void JsonSerializer::Value<TypedUUID>(const char *key,
                                                               TypedUUID &value) {
        std::string serialized = value.ToString();
        Value(key, serialized);
        if (IsReading()) value = TypedUUID::FromString(serialized);
    }

    template <> inline void BinarySerializer::Value<TypedUUID>(const char *key,
                                                                 TypedUUID &value) {
        std::string serialized = value.ToString();
        Value(key, serialized);
        if (IsReading()) value = TypedUUID::FromString(serialized);
    }

    } // namespace axiom
