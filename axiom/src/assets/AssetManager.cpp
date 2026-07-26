#include <axiom/assets/AssetManager.h>

namespace axiom {

    void AssetManager::Init(size_t workerThreadCount) {
        s_WorkerPool = std::make_unique<WorkerPool>(workerThreadCount);
    }

    void AssetManager::Shutdown() {
        s_WorkerPool.reset(); // Destruktor joint alle Worker-Threads
        std::lock_guard lock(s_SlotsMutex);
        s_Slots.clear();
    }

    std::shared_ptr<AssetSlot> AssetManager::GetOrCreateSlot(AssetID id) {
        std::lock_guard lock(s_SlotsMutex);
        auto it = s_Slots.find(id);
        if (it != s_Slots.end()) return it->second;

        auto slot = std::make_shared<AssetSlot>();
        s_Slots[id] = slot;
        return slot;
    }

} // namespace axiom
