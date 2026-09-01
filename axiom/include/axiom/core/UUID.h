#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace axiom {

    // 128-Bit, portabel als zwei uint64_t statt __int128
    // (kein Standard-C++, MSVC unterstuetzt es nicht, std::hash kennt es nicht).
    class UUID {
      public:
        UUID();                                   // erzeugt neue, zufaellige 128-Bit-UUID
        explicit UUID(uint64_t hi, uint64_t lo);   // KEINE implizite Konversion mehr
        explicit UUID(const std::string &hex32);    // aus 32-stelligem Hex-String parsen

        uint64_t Hi() const { return m_hi; }
        uint64_t Lo() const { return m_lo; }

        // Kein operator uint64_t() mehr -- verhindert stille Konversion
        // zwischen Domaenen (Asset/Entity/etc.), siehe TaggedUUID<Tag>.
        std::string ToHexString() const;

        bool operator==(const UUID &other) const {
            return m_hi == other.m_hi && m_lo == other.m_lo;
        }
        bool operator!=(const UUID &other) const { return !(*this == other); }

      private:
        uint64_t m_hi;
        uint64_t m_lo;
    };

} // namespace axiom

namespace std {
    template <> struct hash<axiom::UUID> {
        std::size_t operator()(const axiom::UUID &uuid) const {
            std::size_t h1 = std::hash<uint64_t>{}(uuid.Hi());
            std::size_t h2 = std::hash<uint64_t>{}(uuid.Lo());
            return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
        }
    };
} // namespace std