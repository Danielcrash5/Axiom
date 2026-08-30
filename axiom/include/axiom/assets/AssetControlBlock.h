#pragma once
#include <atomic>
#include <cstdint>
#include <memory>

namespace axiom {

    enum class AssetLoadState : uint8_t {
        Unloaded = 0,
        Queued = 1,
        Loading = 2,
        Loaded = 3,
        Failed = 4,
    };

    // Zentrale Kontroll-Struktur für ein Asset.
    // Ref-Counting via Control-Block (wie shared_ptr/weak_ptr).
    // Erlaubt später Weak-Handles ohne Retrofit.
    struct AssetControlBlock {
        // Ref-Counting
        std::atomic<uint32_t> strongCount{1}; // Start mit 1 für initiale Holder
        std::atomic<uint32_t> weakCount{0};   // Für Weak-Handles, aktuell ungenutzt

        // Zustand
        std::atomic<AssetLoadState> state{AssetLoadState::Unloaded};

        // Eigentliche Asset-Daten
        void *data = nullptr;

        AssetControlBlock() = default;
        virtual ~AssetControlBlock() = default;

        // Verhindert Kopie
        AssetControlBlock(const AssetControlBlock &) = delete;
        AssetControlBlock &operator=(const AssetControlBlock &) = delete;

        void AddStrongRef() { strongCount.fetch_add(1, std::memory_order_relaxed); }

        bool RemoveStrongRef() {
            uint32_t prev = strongCount.fetch_sub(1, std::memory_order_release);
            if (prev == 1) {
                std::atomic_thread_fence(std::memory_order_acquire);
                return true; // Sollte gelöscht werden
            }
            return false;
        }

        uint32_t GetStrongCount() const {
            return strongCount.load(std::memory_order_acquire);
        }

        uint32_t GetWeakCount() const {
            return weakCount.load(std::memory_order_acquire);
        }
    };

} // namespace axiom
