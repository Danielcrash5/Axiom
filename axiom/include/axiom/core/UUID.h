/**
 * @file axiom/include/axiom/core/UUID.h
 * @brief Declares the public interface for the UUID.
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace axiom {

    // 128-Bit, portabel als zwei uint64_t statt __int128
    // (kein Standard-C++, MSVC unterstuetzt es nicht, std::hash kennt es nicht).
    /**
     * @brief Provides a stable identifier for an object.
     * @tparam UUID Template type used by this type.
     */
    class UUID {
      public:
        /**
         * @brief Creates an instance of UUID.
         */
        UUID();                                   // erzeugt neue, zufaellige 128-Bit-UUID
        /**
         * @brief Creates an instance of UUID.
         * @param hi The hi used by this operation.
         * @param lo The lo used by this operation.
         */
        explicit UUID(uint64_t hi, uint64_t lo);   // KEINE implizite Konversion mehr
        /**
         * @brief Creates an instance of UUID.
         * @param hex32 The hex32 used by this operation.
         */
        explicit UUID(const std::string &hex32);    // aus 32-stelligem Hex-String parsen

        /**
         * @brief Returns the high component of the value.
         * @return The result of the operation.
         */
        uint64_t hi() const { return m_hi; }
        /**
         * @brief Returns the low component of the value.
         * @return The result of the operation.
         */
        uint64_t lo() const { return m_lo; }

        // Kein operator uint64_t() mehr -- verhindert stille Konversion
        // zwischen Domaenen (Asset/Entity/etc.), siehe TaggedUUID<Tag>.
        /**
         * @brief Converts this value to hex string.
         * @return The result of the operation.
         */
        std::string toHexString() const;

        bool operator==(const UUID &other) const {
            return m_hi == other.m_hi && m_lo == other.m_lo;
        }
        bool operator!=(const UUID &other) const { return !(*this == other); }

      private:
        /** @brief Stores the hi. */
        uint64_t m_hi;
        /** @brief Stores the lo. */
        uint64_t m_lo;
    };

} // namespace axiom

namespace std {
    template <> struct hash<axiom::UUID> {
        std::size_t operator()(const axiom::UUID &uuid) const {
            std::size_t h1 = std::hash<uint64_t>{}(uuid.hi());
            std::size_t h2 = std::hash<uint64_t>{}(uuid.lo());
            return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
        }
    };
} // namespace std