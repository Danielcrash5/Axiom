/**
 * @file axiom/include/axiom/serialization/Serialization.h
 * @brief Declares the public interface for the serialization.
 */

#pragma once

#include "axiom/assets/VFS.h"

#include <cereal/archives/binary.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

#include <cstdint>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace axiom::serialization {

    // Forward declaration - JSON is only used in implementation files
    // to avoid GCC/nlohmann/json compatibility issues with wide header inclusion

    /**
     * @brief Provides the binary input archive type alias.
     */
    using BinaryInputArchive = cereal::BinaryInputArchive;
    /**
     * @brief Provides the binary output archive type alias.
     */
    using BinaryOutputArchive = cereal::BinaryOutputArchive;
    /**
     * @brief Provides the portable binary input archive type alias.
     */
    using PortableBinaryInputArchive = cereal::PortableBinaryInputArchive;
    /**
     * @brief Provides the portable binary output archive type alias.
     */
    using PortableBinaryOutputArchive = cereal::PortableBinaryOutputArchive;

    namespace detail {

        /**
         * @brief Converts this value to archive bytes.
         * @tparam Archive Template type used by this operation.
         * @tparam T Template type used by this operation.
         * @param value The value used by this operation.
         * @return The result of the operation.
         */
        template <typename Archive, typename T>
        std::vector<std::uint8_t> toArchiveBytes(const T &value) {
            std::ostringstream stream(std::ios::out | std::ios::binary);
            Archive archive(stream);
            archive(value);

            const std::string bytes = stream.str();
            return {bytes.begin(), bytes.end()};
        }

        /**
         * @brief Creates a value from archive bytes data.
         * @tparam Archive Template type used by this operation.
         * @tparam T Template type used by this operation.
         * @param bytes The bytes used by this operation.
         * @return The result of the operation.
         */
        template <typename Archive, typename T>
        T fromArchiveBytes(std::span<const std::uint8_t> bytes) {
            const std::string data(bytes.begin(), bytes.end());
            std::istringstream stream(data, std::ios::in | std::ios::binary);
            Archive archive(stream);

            T value{};
            archive(value);
            return value;
        }

    } // namespace detail

    /**
     * @brief Converts this value to binary.
     * @tparam T Template type used by this operation.
     * @param value The value used by this operation.
     * @return The result of the operation.
     */
    template <typename T> std::vector<std::uint8_t> toBinary(const T &value) {
        return detail::toArchiveBytes<BinaryOutputArchive>(value);
    }

    /**
     * @brief Creates a value from binary data.
     * @tparam T Template type used by this operation.
     * @param bytes The bytes used by this operation.
     * @return The result of the operation.
     */
    template <typename T> T fromBinary(std::span<const std::uint8_t> bytes) {
        return detail::fromArchiveBytes<BinaryInputArchive, T>(bytes);
    }

    /**
     * @brief Creates a value from binary data.
     * @tparam T Template type used by this operation.
     * @param bytes The bytes used by this operation.
     * @return The result of the operation.
     */
    template <typename T> T fromBinary(const std::vector<std::uint8_t> &bytes) {
        return fromBinary<T>(std::span<const std::uint8_t>(bytes));
    }

    /**
     * @brief Converts this value to portable binary.
     * @tparam T Template type used by this operation.
     * @param value The value used by this operation.
     * @return The result of the operation.
     */
    template <typename T>
    std::vector<std::uint8_t> toPortableBinary(const T &value) {
        return detail::toArchiveBytes<PortableBinaryOutputArchive>(value);
    }

    /**
     * @brief Creates a value from portable binary data.
     * @tparam T Template type used by this operation.
     * @param bytes The bytes used by this operation.
     * @return The result of the operation.
     */
    template <typename T>
    T fromPortableBinary(std::span<const std::uint8_t> bytes) {
        return detail::fromArchiveBytes<PortableBinaryInputArchive, T>(bytes);
    }

    /**
     * @brief Creates a value from portable binary data.
     * @tparam T Template type used by this operation.
     * @param bytes The bytes used by this operation.
     * @return The result of the operation.
     */
    template <typename T>
    T fromPortableBinary(const std::vector<std::uint8_t> &bytes) {
        return fromPortableBinary<T>(std::span<const std::uint8_t>(bytes));
    }

    // JSON serialization moved to implementation files to avoid header compilation issues
    // with GCC/nlohmann/json compatibility. Use alternative JSON libraries or direct
    // string serialization if needed.

    /**
     * @brief Attempts to from binary.
     * @tparam T Template type used by this operation.
     * @param bytes The bytes used by this operation.
     * @param[out] outValue Receives the value value.
     * @return The result of the operation.
     */
    template <typename T>
    bool tryFromBinary(std::span<const std::uint8_t> bytes, T &outValue) {
        try {
            outValue = fromBinary<T>(bytes);
            return true;
        } catch (...) {
            return false;
        }
    }

    /**
     * @brief Attempts to from binary.
     * @tparam T Template type used by this operation.
     * @param bytes The bytes used by this operation.
     * @param[out] outValue Receives the value value.
     * @return The result of the operation.
     */
    template <typename T>
    bool tryFromBinary(const std::vector<std::uint8_t> &bytes, T &outValue) {
        return tryFromBinary<T>(std::span<const std::uint8_t>(bytes), outValue);
    }

    /**
     * @brief Attempts to from portable binary.
     * @tparam T Template type used by this operation.
     * @param bytes The bytes used by this operation.
     * @param[out] outValue Receives the value value.
     * @return The result of the operation.
     */
    template <typename T>
    bool tryFromPortableBinary(std::span<const std::uint8_t> bytes,
                               T &outValue) {
        try {
            outValue = fromPortableBinary<T>(bytes);
            return true;
        } catch (...) {
            return false;
        }
    }

    /**
     * @brief Attempts to from portable binary.
     * @tparam T Template type used by this operation.
     * @param bytes The bytes used by this operation.
     * @param[out] outValue Receives the value value.
     * @return The result of the operation.
     */
    template <typename T>
    bool tryFromPortableBinary(const std::vector<std::uint8_t> &bytes,
                               T &outValue) {
        return tryFromPortableBinary<T>(std::span<const std::uint8_t>(bytes),
                                        outValue);
    }

    /**
     * @brief Attempts to from JSON string.
     * @tparam T Template type used by this operation.
     * @param text The text used by this operation.
     * @param[out] outValue Receives the value value.
     * @return The result of the operation.
     */
    template <typename T>
    bool tryfromJsonString(std::string_view text, T &outValue) {
        try {
            outValue = fromJsonString<T>(text);
            return true;
        } catch (...) {
            return false;
        }
    }

    /**
     * @brief Saves binary.
     * @tparam T Template type used by this operation.
     * @param virtualPath The virtual path used by this operation.
     * @param value The value used by this operation.
     * @return The result of the operation.
     */
    template <typename T>
    bool saveBinary(const std::string &virtualPath, const T &value) {
        try {
            return VFS::writeFile(virtualPath, toBinary(value));
        } catch (...) {
            return false;
        }
    }

    /**
     * @brief Loads binary.
     * @tparam T Template type used by this operation.
     * @param virtualPath The virtual path used by this operation.
     * @param[out] outValue Receives the value value.
     * @return The result of the operation.
     */
    template <typename T>
    bool loadBinary(const std::string &virtualPath, T &outValue) {
        std::vector<std::uint8_t> bytes;
        if (!VFS::readFile(virtualPath, bytes))
            return false;

        return tryFromBinary(bytes, outValue);
    }

    /**
     * @brief Saves portable binary.
     * @tparam T Template type used by this operation.
     * @param virtualPath The virtual path used by this operation.
     * @param value The value used by this operation.
     * @return The result of the operation.
     */
    template <typename T>
    bool savePortableBinary(const std::string &virtualPath, const T &value) {
        try {
            return VFS::writeFile(virtualPath, toPortableBinary(value));
        } catch (...) {
            return false;
        }
    }

    /**
     * @brief Loads portable binary.
     * @tparam T Template type used by this operation.
     * @param virtualPath The virtual path used by this operation.
     * @param[out] outValue Receives the value value.
     * @return The result of the operation.
     */
    template <typename T>
    bool loadPortableBinary(const std::string &virtualPath, T &outValue) {
        std::vector<std::uint8_t> bytes;
        if (!VFS::readFile(virtualPath, bytes))
            return false;

        return tryFromPortableBinary(bytes, outValue);
    }

    /**
     * @brief Saves JSON.
     * @tparam T Template type used by this operation.
     * @param virtualPath The virtual path used by this operation.
     * @param value The value used by this operation.
     * @param indent The indent used by this operation.
     * @return The result of the operation.
     */
    template <typename T>
    bool saveJson(const std::string &virtualPath, const T &value,
                  int indent = 4) {
        try {
            return VFS::writeTextFile(virtualPath,
                                      toJsonString(value, indent));
        } catch (...) {
            return false;
        }
    }

    /**
     * @brief Loads JSON.
     * @tparam T Template type used by this operation.
     * @param virtualPath The virtual path used by this operation.
     * @param[out] outValue Receives the value value.
     * @return The result of the operation.
     */
    template <typename T>
    bool loadJson(const std::string &virtualPath, T &outValue) {
        std::string text;
        if (!VFS::readTextFile(virtualPath, text))
            return false;

        return TryfromJsonString(text, outValue);
    }

} // namespace axiom::serialization
