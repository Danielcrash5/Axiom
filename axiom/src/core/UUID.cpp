#include "axiom/core/UUID.h"
#include <array>
#include <iomanip>
#include <random>
#include <sstream>
#include <stdexcept>

namespace axiom {

    namespace {
        // thread_local statt globaler static -- verhindert die Data Race,
        // die die vorherige 64-Bit-Version hatte (mt19937_64::operator()
        // mutiert internen State, mehrere Threads = UB).
        //
        // std::random_device liefert nur ~32 Bit Entropie pro Aufruf --
        // vier Aufrufe zu einem std::seed_seq kombiniert geben dem
        // Generator einen vollen 128-Bit-Startzustand statt eines
        // Seeds mit nur 32 Bit echter Zufaelligkeit.
        std::mt19937_64 &LocalEngine() {
            thread_local std::mt19937_64 engine = [] {
                std::random_device rd;
                std::array<uint32_t, 4> seedData{rd(), rd(), rd(), rd()};
                std::seed_seq seq(seedData.begin(), seedData.end());
                return std::mt19937_64(seq);
            }();
            return engine;
        }

        thread_local std::uniform_int_distribution<uint64_t> s_Dist;
    } // namespace

    UUID::UUID() : m_hi(s_Dist(LocalEngine())), m_lo(s_Dist(LocalEngine())) {}

    UUID::UUID(uint64_t hi, uint64_t lo) : m_hi(hi), m_lo(lo) {}

    UUID::UUID(const std::string &hex32) {
        if (hex32.size() != 32)
            throw std::invalid_argument("UUID: erwarte 32 Hex-Zeichen, bekam " +
                                        std::to_string(hex32.size()));
        m_hi = std::stoull(hex32.substr(0, 16), nullptr, 16);
        m_lo = std::stoull(hex32.substr(16, 16), nullptr, 16);
    }

    std::string UUID::ToHexString() const {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0') << std::setw(16) << m_hi
            << std::setw(16) << m_lo;
        return oss.str();
    }

} // namespace axiom