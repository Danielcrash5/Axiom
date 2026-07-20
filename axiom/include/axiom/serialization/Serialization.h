#pragma once

#include "axiom/assets/VFS.h"

#include <cereal/archives/binary.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <nlohmann/json.hpp>

#include <cstdint>
#include <span>
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

    namespace detail {

        template <typename Archive, typename T>
        std::vector<std::uint8_t> ToArchiveBytes(const T &value) {
            std::ostringstream stream(std::ios::out | std::ios::binary);
            Archive archive(stream);
            archive(value);

            const std::string bytes = stream.str();
            return {bytes.begin(), bytes.end()};
        }

        template <typename Archive, typename T>
        T FromArchiveBytes(std::span<const std::uint8_t> bytes) {
            const std::string data(bytes.begin(), bytes.end());
            std::istringstream stream(data, std::ios::in | std::ios::binary);
            Archive archive(stream);

            T value{};
            archive(value);
            return value;
        }

    } // namespace detail

    template <typename T> std::vector<std::uint8_t> ToBinary(const T &value) {
        return detail::ToArchiveBytes<BinaryOutputArchive>(value);
    }

    template <typename T> T FromBinary(std::span<const std::uint8_t> bytes) {
        return detail::FromArchiveBytes<BinaryInputArchive, T>(bytes);
    }

    template <typename T> T FromBinary(const std::vector<std::uint8_t> &bytes) {
        return FromBinary<T>(std::span<const std::uint8_t>(bytes));
    }

    template <typename T>
    std::vector<std::uint8_t> ToPortableBinary(const T &value) {
        return detail::ToArchiveBytes<PortableBinaryOutputArchive>(value);
    }

    template <typename T>
    T FromPortableBinary(std::span<const std::uint8_t> bytes) {
        return detail::FromArchiveBytes<PortableBinaryInputArchive, T>(bytes);
    }

    template <typename T>
    T FromPortableBinary(const std::vector<std::uint8_t> &bytes) {
        return FromPortableBinary<T>(std::span<const std::uint8_t>(bytes));
    }

    template <typename T>
    std::string ToJsonString(const T &value, int indent = 4) {
        Json json = value;
        return json.dump(indent);
    }

    template <typename T> T FromJsonString(std::string_view text) {
        return Json::parse(text).template get<T>();
    }

    template <typename T>
    bool TryFromBinary(std::span<const std::uint8_t> bytes, T &outValue) {
        try {
            outValue = FromBinary<T>(bytes);
            return true;
        } catch (...) {
            return false;
        }
    }

    template <typename T>
    bool TryFromBinary(const std::vector<std::uint8_t> &bytes, T &outValue) {
        return TryFromBinary<T>(std::span<const std::uint8_t>(bytes), outValue);
    }

    template <typename T>
    bool TryFromPortableBinary(std::span<const std::uint8_t> bytes,
                               T &outValue) {
        try {
            outValue = FromPortableBinary<T>(bytes);
            return true;
        } catch (...) {
            return false;
        }
    }

    template <typename T>
    bool TryFromPortableBinary(const std::vector<std::uint8_t> &bytes,
                               T &outValue) {
        return TryFromPortableBinary<T>(std::span<const std::uint8_t>(bytes),
                                        outValue);
    }

    template <typename T>
    bool TryFromJsonString(std::string_view text, T &outValue) {
        try {
            outValue = FromJsonString<T>(text);
            return true;
        } catch (...) {
            return false;
        }
    }

    template <typename T>
    bool SaveBinary(const std::string &virtualPath, const T &value) {
        try {
            return VFS::WriteFile(virtualPath, ToBinary(value));
        } catch (...) {
            return false;
        }
    }

    template <typename T>
    bool LoadBinary(const std::string &virtualPath, T &outValue) {
        std::vector<std::uint8_t> bytes;
        if (!VFS::ReadFile(virtualPath, bytes))
            return false;

        return TryFromBinary(bytes, outValue);
    }

    template <typename T>
    bool SavePortableBinary(const std::string &virtualPath, const T &value) {
        try {
            return VFS::WriteFile(virtualPath, ToPortableBinary(value));
        } catch (...) {
            return false;
        }
    }

    template <typename T>
    bool LoadPortableBinary(const std::string &virtualPath, T &outValue) {
        std::vector<std::uint8_t> bytes;
        if (!VFS::ReadFile(virtualPath, bytes))
            return false;

        return TryFromPortableBinary(bytes, outValue);
    }

    template <typename T>
    bool SaveJson(const std::string &virtualPath, const T &value,
                  int indent = 4) {
        try {
            return VFS::WriteTextFile(virtualPath,
                                      ToJsonString(value, indent));
        } catch (...) {
            return false;
        }
    }

    template <typename T>
    bool LoadJson(const std::string &virtualPath, T &outValue) {
        std::string text;
        if (!VFS::ReadTextFile(virtualPath, text))
            return false;

        return TryFromJsonString(text, outValue);
    }

} // namespace axiom::serialization
