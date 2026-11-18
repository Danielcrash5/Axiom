/**
 * @file axiom/src/assets/MemoryBudgetPolicy.cpp
 * @brief Implements the memory budget policy module.
 */

#include <axiom/assets/MemoryBudgetPolicy.h>
#include <algorithm>
#include <chrono>

namespace axiom {

    // === LRUMemoryPolicy ===

    LRUMemoryPolicy::LRUMemoryPolicy(size_t budgetBytes) : m_budgetBytes(budgetBytes) {}

    size_t LRUMemoryPolicy::getCurrentUsage() const { return m_currentUsage; }

    size_t LRUMemoryPolicy::getBudget() const { return m_budgetBytes; }

    std::vector<TypedUUID> LRUMemoryPolicy::selectEvictionCandidates(size_t bytesNeeded) {
        std::vector<TypedUUID> candidates;
        size_t freedBytes = 0;

        // Sortiere nach LRU (ältester Zugriff zuerst)
        std::sort(m_loadedAssets.begin(), m_loadedAssets.end(),
                  [](const AssetInfo &a, const AssetInfo &b) {
                      return a.lastAccessTime < b.lastAccessTime;
                  });

        for (const auto &asset : m_loadedAssets) {
            if (freedBytes >= bytesNeeded) {
                break;
            }
            candidates.push_back(asset.uuid);
            freedBytes += asset.sizeBytes;
        }

        return candidates;
    }

    void LRUMemoryPolicy::onAssetLoaded(TypedUUID uuid, size_t sizeBytes) {
        AssetInfo info{uuid, sizeBytes,
                       static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count())};
        m_loadedAssets.push_back(info);
        m_currentUsage += sizeBytes;

        if (m_currentUsage > m_budgetBytes) {
            onBudgetExceeded(m_currentUsage, m_budgetBytes);
        }
    }

    void LRUMemoryPolicy::onAssetUnloaded(TypedUUID uuid, size_t sizeBytes) {
        auto it = std::find_if(m_loadedAssets.begin(), m_loadedAssets.end(),
                               [uuid](const AssetInfo &a) { return a.uuid == uuid; });
        if (it != m_loadedAssets.end()) {
            m_currentUsage -= it->sizeBytes;
            m_loadedAssets.erase(it);
        }
    }

    void LRUMemoryPolicy::onBudgetExceeded(size_t currentUsage, size_t budget) {
        // Log nur für jetzt
        if (currentUsage > budget) {
            // TODO: Benachrichtige AssetManager zur Eviction
        }
    }

    // === PriorityMemoryPolicy ===

    PriorityMemoryPolicy::PriorityMemoryPolicy(size_t budgetBytes) : m_budgetBytes(budgetBytes) {}

    size_t PriorityMemoryPolicy::getCurrentUsage() const { return m_currentUsage; }

    size_t PriorityMemoryPolicy::getBudget() const { return m_budgetBytes; }

    std::vector<TypedUUID> PriorityMemoryPolicy::selectEvictionCandidates(size_t bytesNeeded) {
        std::vector<TypedUUID> candidates;
        size_t freedBytes = 0;

        // Sortiere nach Priorität (niedrigere zuerst) dann nach Größe (größere zuerst)
        std::sort(m_loadedAssets.begin(), m_loadedAssets.end(),
                  [](const AssetInfo &a, const AssetInfo &b) {
                      if (a.priority != b.priority) {
                          return a.priority < b.priority;
                      }
                      return a.sizeBytes > b.sizeBytes;
                  });

        for (const auto &asset : m_loadedAssets) {
            if (freedBytes >= bytesNeeded) {
                break;
            }
            candidates.push_back(asset.uuid);
            freedBytes += asset.sizeBytes;
        }

        return candidates;
    }

    void PriorityMemoryPolicy::onAssetLoaded(TypedUUID uuid, size_t sizeBytes) {
        AssetInfo info{uuid, sizeBytes, 0}; // Standard-Priorität: 0
        m_loadedAssets.push_back(info);
        m_currentUsage += sizeBytes;
    }

    void PriorityMemoryPolicy::onAssetUnloaded(TypedUUID uuid, size_t sizeBytes) {
        auto it = std::find_if(m_loadedAssets.begin(), m_loadedAssets.end(),
                               [uuid](const AssetInfo &a) { return a.uuid == uuid; });
        if (it != m_loadedAssets.end()) {
            m_currentUsage -= it->sizeBytes;
            m_loadedAssets.erase(it);
        }
    }

    void PriorityMemoryPolicy::setAssetPriority(TypedUUID uuid, int priority) {
        auto it = std::find_if(m_loadedAssets.begin(), m_loadedAssets.end(),
                               [uuid](const AssetInfo &a) { return a.uuid == uuid; });
        if (it != m_loadedAssets.end()) {
            it->priority = priority;
        }
    }

} // namespace axiom
