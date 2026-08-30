#pragma once
#include <axiom/assets/TypedUUID.h>
#include <cstddef>
#include <vector>

namespace axiom {

    // Memory Budget Policy für Eviction von Assets bei Budget-Überschreitung
    class IMemoryBudgetPolicy {
      public:
        virtual ~IMemoryBudgetPolicy() = default;

        // Gibt aktuelle Speichernutzung zurück
        virtual size_t GetCurrentUsage() const = 0;

        // Gibt das verfügbare Budget zurück
        virtual size_t GetBudget() const = 0;

        // Gibt Kandidaten für Eviction zurück (UUIDs)
        // wenn bytesNeeded zusätzliche Bytes benötigt werden
        virtual std::vector<TypedUUID> SelectEvictionCandidates(size_t bytesNeeded) = 0;

        // Optional: Wird aufgerufen, wenn ein Asset geladen wird
        virtual void OnAssetLoaded(TypedUUID uuid, size_t sizeBytes) {}

        // Optional: Wird aufgerufen, wenn ein Asset entladen wird
        virtual void OnAssetUnloaded(TypedUUID uuid, size_t sizeBytes) {}

        // Optional: Callback wenn Budget überschritten
        virtual void OnBudgetExceeded(size_t currentUsage, size_t budget) {}
    };

    // Einfache LRU-basierte Policy
    class LRUMemoryPolicy : public IMemoryBudgetPolicy {
      public:
        LRUMemoryPolicy(size_t budgetBytes);

        size_t GetCurrentUsage() const override;
        size_t GetBudget() const override;
        std::vector<TypedUUID> SelectEvictionCandidates(size_t bytesNeeded) override;
        void OnAssetLoaded(TypedUUID uuid, size_t sizeBytes) override;
        void OnAssetUnloaded(TypedUUID uuid, size_t sizeBytes) override;
        void OnBudgetExceeded(size_t currentUsage, size_t budget) override;

      private:
        struct AssetInfo {
            TypedUUID uuid;
            size_t sizeBytes;
            uint64_t lastAccessTime;
        };

        size_t m_BudgetBytes;
        size_t m_CurrentUsage = 0;
        std::vector<AssetInfo> m_LoadedAssets;
    };

    // Priority-basierte Policy (z.B. für Streaming)
    class PriorityMemoryPolicy : public IMemoryBudgetPolicy {
      public:
        PriorityMemoryPolicy(size_t budgetBytes);

        size_t GetCurrentUsage() const override;
        size_t GetBudget() const override;
        std::vector<TypedUUID> SelectEvictionCandidates(size_t bytesNeeded) override;
        void OnAssetLoaded(TypedUUID uuid, size_t sizeBytes) override;
        void OnAssetUnloaded(TypedUUID uuid, size_t sizeBytes) override;

        // Setze Priorität eines Assets (höher = später evicted)
        void SetAssetPriority(TypedUUID uuid, int priority);

      private:
        struct AssetInfo {
            TypedUUID uuid;
            size_t sizeBytes;
            int priority = 0;
        };

        size_t m_BudgetBytes;
        size_t m_CurrentUsage = 0;
        std::vector<AssetInfo> m_LoadedAssets;
    };

} // namespace axiom
