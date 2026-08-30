#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace axiom {

    // Polymorphes Serialisierungs-Interface
    // Identische API für JSON und Binary - Asset-Typen wissen nicht, welche verwendet wird
    class ISerializer {
      public:
        virtual ~ISerializer() = default;

        enum class Mode { Reading = 0, Writing = 1 };

        virtual Mode GetMode() const = 0;
        virtual bool IsReading() const { return GetMode() == Mode::Reading; }
        virtual bool IsWriting() const { return GetMode() == Mode::Writing; }

        // === Basis-Typen ===
        virtual bool Value(const char *key, bool &value) = 0;
        virtual bool Value(const char *key, int8_t &value) = 0;
        virtual bool Value(const char *key, uint8_t &value) = 0;
        virtual bool Value(const char *key, int16_t &value) = 0;
        virtual bool Value(const char *key, uint16_t &value) = 0;
        virtual bool Value(const char *key, int32_t &value) = 0;
        virtual bool Value(const char *key, uint32_t &value) = 0;
        virtual bool Value(const char *key, int64_t &value) = 0;
        virtual bool Value(const char *key, uint64_t &value) = 0;
        virtual bool Value(const char *key, float &value) = 0;
        virtual bool Value(const char *key, double &value) = 0;
        virtual bool Value(const char *key, std::string &value) = 0;

        // === Container ===
        virtual bool BeginArray(const char *key, uint32_t &count) = 0;
        virtual bool EndArray() = 0;
        virtual bool BeginObject(const char *key) = 0;
        virtual bool EndObject() = 0;

        // === Hilfsmittel ===
        virtual bool HasKey(const char *key) const = 0;
        virtual std::string GetDebugInfo() const = 0;
    };

    // JSON-Serialisierung (für .axmeta, editierbare Dateien)
    class JsonSerializer : public ISerializer {
      public:
        JsonSerializer();
        ~JsonSerializer() override;

        // Laden aus JSON-Text
        bool LoadFromString(const std::string &jsonText);

        // Speichern als JSON-Text
        std::string SaveToString(bool pretty = true) const;

        // ISerializer Interface
        Mode GetMode() const override;
        bool Value(const char *key, bool &value) override;
        bool Value(const char *key, int8_t &value) override;
        bool Value(const char *key, uint8_t &value) override;
        bool Value(const char *key, int16_t &value) override;
        bool Value(const char *key, uint16_t &value) override;
        bool Value(const char *key, int32_t &value) override;
        bool Value(const char *key, uint32_t &value) override;
        bool Value(const char *key, int64_t &value) override;
        bool Value(const char *key, uint64_t &value) override;
        bool Value(const char *key, float &value) override;
        bool Value(const char *key, double &value) override;
        bool Value(const char *key, std::string &value) override;

        bool BeginArray(const char *key, uint32_t &count) override;
        bool EndArray() override;
        bool BeginObject(const char *key) override;
        bool EndObject() override;
        bool HasKey(const char *key) const override;
        std::string GetDebugInfo() const override;

      private:
        void *m_Json = nullptr; // nlohmann::json*
        Mode m_Mode = Mode::Reading;
    };

    // Binary-Serialisierung (für .axasset, nicht editierbar)
    class BinarySerializer : public ISerializer {
      public:
        BinarySerializer();
        ~BinarySerializer() override;

        // Laden aus Binary-Daten
        bool LoadFromBytes(const std::vector<uint8_t> &data);

        // Speichern als Binary-Daten
        std::vector<uint8_t> SaveToBytes() const;

        // ISerializer Interface
        Mode GetMode() const override;
        bool Value(const char *key, bool &value) override;
        bool Value(const char *key, int8_t &value) override;
        bool Value(const char *key, uint8_t &value) override;
        bool Value(const char *key, int16_t &value) override;
        bool Value(const char *key, uint16_t &value) override;
        bool Value(const char *key, int32_t &value) override;
        bool Value(const char *key, uint32_t &value) override;
        bool Value(const char *key, int64_t &value) override;
        bool Value(const char *key, uint64_t &value) override;
        bool Value(const char *key, float &value) override;
        bool Value(const char *key, double &value) override;
        bool Value(const char *key, std::string &value) override;

        bool BeginArray(const char *key, uint32_t &count) override;
        bool EndArray() override;
        bool BeginObject(const char *key) override;
        bool EndObject() override;
        bool HasKey(const char *key) const override;
        std::string GetDebugInfo() const override;

      private:
        std::vector<uint8_t> m_Buffer;
        size_t m_Position = 0;
        Mode m_Mode = Mode::Reading;

        void WriteBytes(const uint8_t *data, size_t size);
        bool ReadBytes(uint8_t *data, size_t size);
    };

} // namespace axiom
