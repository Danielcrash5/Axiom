#pragma once
#include <cstdint>
#include <random>

namespace axiom {

    class UUID {
    public:
        UUID();
        UUID(uint64_t uuid);

        operator uint64_t() const { return m_UUID; }

        bool operator==(const UUID& other) const { return m_UUID == other.m_UUID; }
        bool operator!=(const UUID& other) const { return !(*this == other); }

    private:
        uint64_t m_UUID;
    };

} // namespace axiom

namespace std {
    template<>
    struct hash<axiom::UUID> {
        std::size_t operator()(const axiom::UUID& uuid) const {
            return static_cast<uint64_t>(uuid);
        }
    };
}
