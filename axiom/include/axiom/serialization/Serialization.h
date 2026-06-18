#pragma once

#include <cereal/archives/binary.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <nlohmann/json.hpp>

#include <cstdint>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace axiom::serialization {

    using Json = nlohmann::json;

    using BinaryInputArchive = cereal::BinaryInputArchive;
    using BinaryOutputArchive = cereal::BinaryOutputArchive;
    using PortableBinaryInputArchive = cereal::PortableBinaryInputArchive;
    using PortableBinaryOutputArchive = cereal::PortableBinaryOutputArchive;

    template<typename T>
    std::vector<std::uint8_t> ToBinary(const T& value) {
        std::ostringstream stream(std::ios::binary);
        BinaryOutputArchive archive(stream);
        archive(value);

        const std::string bytes = stream.str();
        return {bytes.begin(), bytes.end()};
    }

    template<typename T>
    T FromBinary(const std::vector<std::uint8_t>& bytes) {
        const std::string data(bytes.begin(), bytes.end());
        std::istringstream stream(data, std::ios::binary);
        BinaryInputArchive archive(stream);

        T value{};
        archive(value);
        return value;
    }

    template<typename T>
    std::vector<std::uint8_t> ToPortableBinary(const T& value) {
        std::ostringstream stream(std::ios::binary);
        PortableBinaryOutputArchive archive(stream);
        archive(value);

        const std::string bytes = stream.str();
        return {bytes.begin(), bytes.end()};
    }

    template<typename T>
    T FromPortableBinary(const std::vector<std::uint8_t>& bytes) {
        const std::string data(bytes.begin(), bytes.end());
        std::istringstream stream(data, std::ios::binary);
        PortableBinaryInputArchive archive(stream);

        T value{};
        archive(value);
        return value;
    }

    template<typename T>
    std::string ToJsonString(const T& value, int indent = 4) {
        Json json = value;
        return json.dump(indent);
    }

    template<typename T>
    T FromJsonString(std::string_view text) {
        return Json::parse(text).template get<T>();
    }

} // namespace axiom::serialization
