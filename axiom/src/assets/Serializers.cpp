#include <axiom/assets/Serializers.h>
#include <nlohmann/json.hpp>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <sstream>

namespace axiom {
    using json = nlohmann::json;

    // === JsonSerializer ===

    JsonSerializer::JsonSerializer() { m_Json = new json(); }

    JsonSerializer::~JsonSerializer() { delete static_cast<json *>(m_Json); }

    bool JsonSerializer::LoadFromString(const std::string &jsonText) {
        try {
            *static_cast<json *>(m_Json) = json::parse(jsonText);
            m_Mode = Mode::Reading;
            return true;
        } catch (const std::exception &e) {
            return false;
        }
    }

    std::string JsonSerializer::SaveToString(bool pretty) const {
        auto j = static_cast<json *>(m_Json);
        return pretty ? j->dump(2) : j->dump();
    }

    JsonSerializer::Mode JsonSerializer::GetMode() const { return m_Mode; }

    bool JsonSerializer::Value(const char *key, bool &value) {
        auto j = static_cast<json *>(m_Json);
        if (IsReading()) {
            if (j->contains(key)) {
                value = j->at(key).get<bool>();
                return true;
            }
            return false;
        } else {
            (*j)[key] = value;
            return true;
        }
    }

    bool JsonSerializer::Value(const char *key, int8_t &value) {
        auto j = static_cast<json *>(m_Json);
        if (IsReading()) {
            if (j->contains(key)) {
                value = j->at(key).get<int8_t>();
                return true;
            }
            return false;
        } else {
            (*j)[key] = value;
            return true;
        }
    }

    bool JsonSerializer::Value(const char *key, uint8_t &value) {
        auto j = static_cast<json *>(m_Json);
        if (IsReading()) {
            if (j->contains(key)) {
                value = j->at(key).get<uint8_t>();
                return true;
            }
            return false;
        } else {
            (*j)[key] = value;
            return true;
        }
    }

    bool JsonSerializer::Value(const char *key, int16_t &value) {
        auto j = static_cast<json *>(m_Json);
        if (IsReading()) {
            if (j->contains(key)) {
                value = j->at(key).get<int16_t>();
                return true;
            }
            return false;
        } else {
            (*j)[key] = value;
            return true;
        }
    }

    bool JsonSerializer::Value(const char *key, uint16_t &value) {
        auto j = static_cast<json *>(m_Json);
        if (IsReading()) {
            if (j->contains(key)) {
                value = j->at(key).get<uint16_t>();
                return true;
            }
            return false;
        } else {
            (*j)[key] = value;
            return true;
        }
    }

    bool JsonSerializer::Value(const char *key, int32_t &value) {
        auto j = static_cast<json *>(m_Json);
        if (IsReading()) {
            if (j->contains(key)) {
                value = j->at(key).get<int32_t>();
                return true;
            }
            return false;
        } else {
            (*j)[key] = value;
            return true;
        }
    }

    bool JsonSerializer::Value(const char *key, uint32_t &value) {
        auto j = static_cast<json *>(m_Json);
        if (IsReading()) {
            if (j->contains(key)) {
                value = j->at(key).get<uint32_t>();
                return true;
            }
            return false;
        } else {
            (*j)[key] = value;
            return true;
        }
    }

    bool JsonSerializer::Value(const char *key, int64_t &value) {
        auto j = static_cast<json *>(m_Json);
        if (IsReading()) {
            if (j->contains(key)) {
                value = j->at(key).get<int64_t>();
                return true;
            }
            return false;
        } else {
            (*j)[key] = value;
            return true;
        }
    }

    bool JsonSerializer::Value(const char *key, uint64_t &value) {
        auto j = static_cast<json *>(m_Json);
        if (IsReading()) {
            if (j->contains(key)) {
                value = j->at(key).get<uint64_t>();
                return true;
            }
            return false;
        } else {
            (*j)[key] = value;
            return true;
        }
    }

    bool JsonSerializer::Value(const char *key, float &value) {
        auto j = static_cast<json *>(m_Json);
        if (IsReading()) {
            if (j->contains(key)) {
                value = j->at(key).get<float>();
                return true;
            }
            return false;
        } else {
            (*j)[key] = value;
            return true;
        }
    }

    bool JsonSerializer::Value(const char *key, double &value) {
        auto j = static_cast<json *>(m_Json);
        if (IsReading()) {
            if (j->contains(key)) {
                value = j->at(key).get<double>();
                return true;
            }
            return false;
        } else {
            (*j)[key] = value;
            return true;
        }
    }

    bool JsonSerializer::Value(const char *key, std::string &value) {
        auto j = static_cast<json *>(m_Json);
        if (IsReading()) {
            if (j->contains(key)) {
                value = j->at(key).get<std::string>();
                return true;
            }
            return false;
        } else {
            (*j)[key] = value;
            return true;
        }
    }

    bool JsonSerializer::BeginArray(const char *key, uint32_t &count) {
        auto j = static_cast<json *>(m_Json);
        if (IsReading()) {
            if (j->contains(key) && j->at(key).is_array()) {
                count = j->at(key).size();
                // TODO: Array-Navigation
                return true;
            }
            return false;
        } else {
            (*j)[key] = json::array();
            return true;
        }
    }

    bool JsonSerializer::EndArray() {
        // TODO: Array-Navigation
        return true;
    }

    bool JsonSerializer::BeginObject(const char *key) {
        auto j = static_cast<json *>(m_Json);
        if (IsWriting()) {
            (*j)[key] = json::object();
            // TODO: Object-Navigation
        }
        return true;
    }

    bool JsonSerializer::EndObject() {
        // TODO: Object-Navigation
        return true;
    }

    bool JsonSerializer::HasKey(const char *key) const {
        auto j = static_cast<const json *>(m_Json);
        return j->contains(key);
    }

    std::string JsonSerializer::GetDebugInfo() const {
        auto j = static_cast<const json *>(m_Json);
        return j->dump();
    }

    // === BinarySerializer ===

    BinarySerializer::BinarySerializer() {}

    BinarySerializer::~BinarySerializer() {}

    bool BinarySerializer::LoadFromBytes(const std::vector<uint8_t> &data) {
        m_Buffer = data;
        m_Position = 0;
        m_Mode = Mode::Reading;
        return true;
    }

    std::vector<uint8_t> BinarySerializer::SaveToBytes() const { return m_Buffer; }

    BinarySerializer::Mode BinarySerializer::GetMode() const { return m_Mode; }

    void BinarySerializer::WriteBytes(const uint8_t *data, size_t size) {
        if (IsWriting()) {
            m_Buffer.insert(m_Buffer.end(), data, data + size);
        }
    }

    bool BinarySerializer::ReadBytes(uint8_t *data, size_t size) {
        if (m_Position + size > m_Buffer.size()) {
            return false;
        }
        std::copy(m_Buffer.begin() + m_Position, m_Buffer.begin() + m_Position + size, data);
        m_Position += size;
        return true;
    }

#define BINARY_VALUE_IMPL(type)                                                  \
    bool BinarySerializer::Value(const char *key, type &value) {                 \
        if (IsReading()) {                                                       \
            return ReadBytes(reinterpret_cast<uint8_t *>(&value), sizeof(type)); \
        } else {                                                                 \
            WriteBytes(reinterpret_cast<const uint8_t *>(&value), sizeof(type)); \
            return true;                                                         \
        }                                                                        \
    }

    BINARY_VALUE_IMPL(bool)
    BINARY_VALUE_IMPL(int8_t)
    BINARY_VALUE_IMPL(uint8_t)
    BINARY_VALUE_IMPL(int16_t)
    BINARY_VALUE_IMPL(uint16_t)
    BINARY_VALUE_IMPL(int32_t)
    BINARY_VALUE_IMPL(uint32_t)
    BINARY_VALUE_IMPL(int64_t)
    BINARY_VALUE_IMPL(uint64_t)
    BINARY_VALUE_IMPL(float)
    BINARY_VALUE_IMPL(double)

#undef BINARY_VALUE_IMPL

    bool BinarySerializer::Value(const char *key, std::string &value) {
        if (IsReading()) {
            uint32_t length = 0;
            if (!ReadBytes(reinterpret_cast<uint8_t *>(&length), sizeof(uint32_t))) {
                return false;
            }
            value.resize(length);
            return ReadBytes(reinterpret_cast<uint8_t *>(value.data()), length);
        } else {
            uint32_t length = value.length();
            WriteBytes(reinterpret_cast<const uint8_t *>(&length), sizeof(uint32_t));
            WriteBytes(reinterpret_cast<const uint8_t *>(value.data()), length);
            return true;
        }
    }

    bool BinarySerializer::BeginArray(const char *key, uint32_t &count) {
        if (IsReading()) {
            return ReadBytes(reinterpret_cast<uint8_t *>(&count), sizeof(uint32_t));
        } else {
            WriteBytes(reinterpret_cast<const uint8_t *>(&count), sizeof(uint32_t));
            return true;
        }
    }

    bool BinarySerializer::EndArray() { return true; }

    bool BinarySerializer::BeginObject(const char *key) { return true; }

    bool BinarySerializer::EndObject() { return true; }

    bool BinarySerializer::HasKey(const char *key) const { return true; }

    std::string BinarySerializer::GetDebugInfo() const {
        return "BinarySerializer (position: " + std::to_string(m_Position) +
               ", size: " + std::to_string(m_Buffer.size()) + ")";
    }

} // namespace axiom
