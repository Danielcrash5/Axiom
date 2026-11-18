/**
 * @file axiom/src/assets/Serializers.cpp
 * @brief Implements the serializers module.
 */

#include <axiom/serialization/Serializers.h>
#include <nlohmann/json.hpp>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <sstream>

namespace axiom {
    using json = nlohmann::json;

    // === JsonSerializer ===

    JsonSerializer::JsonSerializer() { m_json = new json(); }

    JsonSerializer::~JsonSerializer() { delete static_cast<json *>(m_json); }

    bool JsonSerializer::loadFromString(const std::string &jsonText) {
        try {
            *static_cast<json *>(m_json) = json::parse(jsonText);
            m_mode = Mode::reading;
            return true;
        } catch (const std::exception &e) {
            return false;
        }
    }

    std::string JsonSerializer::saveToString(bool pretty) const {
        auto j = static_cast<json *>(m_json);
        return pretty ? j->dump(2) : j->dump();
    }

    JsonSerializer::Mode JsonSerializer::getMode() const { return m_mode; }

    bool JsonSerializer::value(const char *key, bool &value) {
        auto j = static_cast<json *>(m_json);
        if (isReading()) {
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

    bool JsonSerializer::value(const char *key, int8_t &value) {
        auto j = static_cast<json *>(m_json);
        if (isReading()) {
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

    bool JsonSerializer::value(const char *key, uint8_t &value) {
        auto j = static_cast<json *>(m_json);
        if (isReading()) {
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

    bool JsonSerializer::value(const char *key, int16_t &value) {
        auto j = static_cast<json *>(m_json);
        if (isReading()) {
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

    bool JsonSerializer::value(const char *key, uint16_t &value) {
        auto j = static_cast<json *>(m_json);
        if (isReading()) {
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

    bool JsonSerializer::value(const char *key, int32_t &value) {
        auto j = static_cast<json *>(m_json);
        if (isReading()) {
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

    bool JsonSerializer::value(const char *key, uint32_t &value) {
        auto j = static_cast<json *>(m_json);
        if (isReading()) {
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

    bool JsonSerializer::value(const char *key, int64_t &value) {
        auto j = static_cast<json *>(m_json);
        if (isReading()) {
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

    bool JsonSerializer::value(const char *key, uint64_t &value) {
        auto j = static_cast<json *>(m_json);
        if (isReading()) {
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

    bool JsonSerializer::value(const char *key, float &value) {
        auto j = static_cast<json *>(m_json);
        if (isReading()) {
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

    bool JsonSerializer::value(const char *key, double &value) {
        auto j = static_cast<json *>(m_json);
        if (isReading()) {
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

    bool JsonSerializer::value(const char *key, std::string &value) {
        auto j = static_cast<json *>(m_json);
        if (isReading()) {
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

    bool JsonSerializer::beginArray(const char *key, uint32_t &count) {
        auto j = static_cast<json *>(m_json);
        if (isReading()) {
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

    bool JsonSerializer::endArray() {
        // TODO: Array-Navigation
        return true;
    }

    bool JsonSerializer::beginObject(const char *key) {
        auto j = static_cast<json *>(m_json);
        if (isWriting()) {
            (*j)[key] = json::object();
            // TODO: Object-Navigation
        }
        return true;
    }

    bool JsonSerializer::endObject() {
        // TODO: Object-Navigation
        return true;
    }

    bool JsonSerializer::hasKey(const char *key) const {
        auto j = static_cast<const json *>(m_json);
        return j->contains(key);
    }

    std::string JsonSerializer::getDebugInfo() const {
        auto j = static_cast<const json *>(m_json);
        return j->dump();
    }

    // === BinarySerializer ===

    BinarySerializer::BinarySerializer() {}

    BinarySerializer::~BinarySerializer() {}

    bool BinarySerializer::loadFromBytes(const std::vector<uint8_t> &data) {
        m_buffer = data;
        m_position = 0;
        m_mode = Mode::reading;
        return true;
    }

    std::vector<uint8_t> BinarySerializer::saveToBytes() const { return m_buffer; }

    BinarySerializer::Mode BinarySerializer::getMode() const { return m_mode; }

    void BinarySerializer::writeBytes(const uint8_t *data, size_t size) {
        if (isWriting()) {
            m_buffer.insert(m_buffer.end(), data, data + size);
        }
    }

    bool BinarySerializer::readBytes(uint8_t *data, size_t size) {
        if (m_position + size > m_buffer.size()) {
            return false;
        }
        std::copy(m_buffer.begin() + m_position, m_buffer.begin() + m_position + size, data);
        m_position += size;
        return true;
    }

/** @brief Provides the BINARY_VALUE_IMPL macro. */
#define BINARY_VALUE_IMPL(type)                                                  \
    bool BinarySerializer::value(const char *key, type &value) {                 \
        if (isReading()) {                                                       \
            return readBytes(reinterpret_cast<uint8_t *>(&value), sizeof(type)); \
        } else {                                                                 \
            writeBytes(reinterpret_cast<const uint8_t *>(&value), sizeof(type)); \
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

    bool BinarySerializer::value(const char *key, std::string &value) {
        if (isReading()) {
            uint32_t length = 0;
            if (!readBytes(reinterpret_cast<uint8_t *>(&length), sizeof(uint32_t))) {
                return false;
            }
            value.resize(length);
            return readBytes(reinterpret_cast<uint8_t *>(value.data()), length);
        } else {
            uint32_t length = value.length();
            writeBytes(reinterpret_cast<const uint8_t *>(&length), sizeof(uint32_t));
            writeBytes(reinterpret_cast<const uint8_t *>(value.data()), length);
            return true;
        }
    }

    bool BinarySerializer::beginArray(const char *key, uint32_t &count) {
        if (isReading()) {
            return readBytes(reinterpret_cast<uint8_t *>(&count), sizeof(uint32_t));
        } else {
            writeBytes(reinterpret_cast<const uint8_t *>(&count), sizeof(uint32_t));
            return true;
        }
    }

    bool BinarySerializer::endArray() { return true; }

    bool BinarySerializer::beginObject(const char *key) { return true; }

    bool BinarySerializer::endObject() { return true; }

    bool BinarySerializer::hasKey(const char *key) const { return true; }

    std::string BinarySerializer::getDebugInfo() const {
        return "BinarySerializer (position: " + std::to_string(m_position) +
               ", size: " + std::to_string(m_buffer.size()) + ")";
    }

} // namespace axiom
