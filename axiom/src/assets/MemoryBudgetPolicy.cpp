#include <axiom/assets/MemoryBudgetPolicy.h>
#include <algorithm>
#include <chrono>

namespace axiom {

    // === LRUMemoryPolicy ===

    LRUMemoryPolicy::LRUMemoryPolicy(size_t budgetBytes) : m_BudgetBytes(budgetBytes) {}

    size_t LRUMemoryPolicy::GetCurrentUsage() const { return m_CurrentUsage; }

    size_t LRUMemoryPolicy::GetBudget() const { return m_BudgetBytes; }

    std::vector<TypedUUID> LRUMemoryPolicy::SelectEvictionCandidates(size_t bytesNeeded) {
        std::vector<TypedUUID> candidates;
        size_t freedBytes = 0;

        // Sortiere nach LRU (ältester Zugriff zuerst)
        std::sort(m_LoadedAssets.begin(), m_LoadedAssets.end(),
                  [](const AssetInfo &a, const AssetInfo &b) {
                      return a.lastAccessTime < b.lastAccessTime;
                  });

        for (const auto &asset : m_LoadedAssets) {
            if (freedBytes >= bytesNeeded) {
                break;
            }
            candidates.push_back(asset.uuid);
            freedBytes += asset.sizeBytes;
        }

        return candidates;
    }

    void LRUMemoryPolicy::OnAssetLoaded(TypedUUID uuid, size_t sizeBytes) {
        AssetInfo info{uuid, sizeBytes,
                       std::chrono::high_resolution_clock::now().time_since_epoch().count()};
        m_LoadedAssets.push_back(info);
        m_CurrentUsage += sizeBytes;

        if (m_CurrentUsage > m_BudgetBytes) {
            OnBudgetExceeded(m_CurrentUsage, m_BudgetBytes);
        }
    }

    void LRUMemoryPolicy::OnAssetUnloaded(TypedUUID uuid, size_t sizeBytes) {
        auto it = std::find_if(m_LoadedAssets.begin(), m_LoadedAssets.end(),
                               [uuid](const AssetInfo &a) { return a.uuid == uuid; });
        if (it != m_LoadedAssets.end()) {
            m_CurrentUsage -= it->sizeBytes;
            m_LoadedAssets.erase(it);
        }
    }

    void LRUMemoryPolicy::OnBudgetExceeded(size_t currentUsage, size_t budget) {
        // Log nur für jetzt
        if (currentUsage > budget) {
            // TODO: Benachrichtige AssetManager zur Eviction
        }
    }

    // === PriorityMemoryPolicy ===

    PriorityMemoryPolicy::PriorityMemoryPolicy(size_t budgetBytes) : m_BudgetBytes(budgetBytes) {}

    size_t PriorityMemoryPolicy::GetCurrentUsage() const { return m_CurrentUsage; }

    size_t PriorityMemoryPolicy::GetBudget() const { return m_BudgetBytes; }

    std::vector<TypedUUID> PriorityMemoryPolicy::SelectEvictionCandidates(size_t bytesNeeded) {
        std::vector<TypedUUID> candidates;
        size_t freedBytes = 0;

        // Sortiere nach Priorität (niedrigere zuerst) dann nach Größe (größere zuerst)
        std::sort(m_LoadedAssets.begin(), m_LoadedAssets.end(),
                  [](const AssetInfo &a, const AssetInfo &b) {
                      if (a.priority != b.priority) {
                          return a.priority < b.priority;
                      }
                      return a.sizeBytes > b.sizeBytes;
                  });

        for (const auto &asset : m_LoadedAssets) {
            if (freedBytes >= bytesNeeded) {
                break;
            }
            candidates.push_back(asset.uuid);
            freedBytes += asset.sizeBytes;
        }

        return candidates;
    }

    void PriorityMemoryPolicy::OnAssetLoaded(TypedUUID uuid, size_t sizeBytes) {
        AssetInfo info{uuid, sizeBytes, 0}; // Standard-Priorität: 0
        m_LoadedAssets.push_back(info);
        m_CurrentUsage += sizeBytes;
    }

    void PriorityMemoryPolicy::OnAssetUnloaded(TypedUUID uuid, size_t sizeBytes) {
        auto it = std::find_if(m_LoadedAssets.begin(), m_LoadedAssets.end(),
                               [uuid](const AssetInfo &a) { return a.uuid == uuid; });
        if (it != m_LoadedAssets.end()) {
            m_CurrentUsage -= it->sizeBytes;
            m_LoadedAssets.erase(it);
        }
    }

    void PriorityMemoryPolicy::SetAssetPriority(TypedUUID uuid, int priority) {
        auto it = std::find_if(m_LoadedAssets.begin(), m_LoadedAssets.end(),
                               [uuid](const AssetInfo &a) { return a.uuid == uuid; });
        if (it != m_LoadedAssets.end()) {
            it->priority = priority;
        }
    }

} // namespace axiom
