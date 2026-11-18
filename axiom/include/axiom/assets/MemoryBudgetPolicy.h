/**
 * @file axiom/include/axiom/assets/MemoryBudgetPolicy.h
 * @brief Declares the public interface for the memory budget policy.
 */

#pragma once
#include <axiom/assets/TypedUUID.h>
#include <cstddef>
#include <vector>

namespace axiom {

    // Memory Budget Policy für Eviction von Assets bei Budget-Überschreitung
    /**
     * @brief Defines the interface for the memory budget policy.
     * @tparam IMemoryBudgetPolicy Template type used by this type.
     */
    class IMemoryBudgetPolicy {
      public:
        /**
         * @brief Releases resources owned by IMemoryBudgetPolicy.
         */
        virtual ~IMemoryBudgetPolicy() = default;

        // Gibt aktuelle Speichernutzung zurück
        /**
         * @brief Returns the current usage.
         * @return The result of the operation.
         */
        virtual size_t getCurrentUsage() const = 0;

        // Gibt das verfügbare Budget zurück
        /**
         * @brief Returns the budget.
         * @return The result of the operation.
         */
        virtual size_t getBudget() const = 0;

        // Gibt Kandidaten für Eviction zurück (UUIDs)
        // wenn bytesNeeded zusätzliche Bytes benötigt werden
        /**
         * @brief Selects assets eligible for eviction.
         * @param bytesNeeded The bytes needed used by this operation.
         * @return The result of the operation.
         */
        virtual std::vector<TypedUUID> selectEvictionCandidates(size_t bytesNeeded) = 0;

        // Optional: Wird aufgerufen, wenn ein Asset geladen wird
        /**
         * @brief Handles the asset loaded callback.
         * @param uuid The UUID used by this operation.
         * @param sizeBytes The size bytes used by this operation.
         */
        virtual void onAssetLoaded(TypedUUID uuid, size_t sizeBytes) {}

        // Optional: Wird aufgerufen, wenn ein Asset entladen wird
        /**
         * @brief Handles the asset unloaded callback.
         * @param uuid The UUID used by this operation.
         * @param sizeBytes The size bytes used by this operation.
         */
        virtual void onAssetUnloaded(TypedUUID uuid, size_t sizeBytes) {}

        // Optional: Callback wenn Budget überschritten
        /**
         * @brief Handles the budget exceeded callback.
         * @param currentUsage The current usage used by this operation.
         * @param budget The budget used by this operation.
         */
        virtual void onBudgetExceeded(size_t currentUsage, size_t budget) {}
    };

    // Einfache LRU-basierte Policy
    /**
     * @brief Provides the lru memory policy functionality.
     * @tparam LRUMemoryPolicy Template type used by this type.
     */
    class LRUMemoryPolicy : public IMemoryBudgetPolicy {
      public:
        /**
         * @brief Creates an instance of lru memory policy.
         * @param budgetBytes The budget bytes used by this operation.
         */
        LRUMemoryPolicy(size_t budgetBytes);

        /**
         * @brief Returns the current usage.
         * @return The result of the operation.
         */
        size_t getCurrentUsage() const override;
        /**
         * @brief Returns the budget.
         * @return The result of the operation.
         */
        size_t getBudget() const override;
        /**
         * @brief Selects assets eligible for eviction.
         * @param bytesNeeded The bytes needed used by this operation.
         * @return The result of the operation.
         */
        std::vector<TypedUUID> selectEvictionCandidates(size_t bytesNeeded) override;
        /**
         * @brief Handles the asset loaded callback.
         * @param uuid The UUID used by this operation.
         * @param sizeBytes The size bytes used by this operation.
         */
        void onAssetLoaded(TypedUUID uuid, size_t sizeBytes) override;
        /**
         * @brief Handles the asset unloaded callback.
         * @param uuid The UUID used by this operation.
         * @param sizeBytes The size bytes used by this operation.
         */
        void onAssetUnloaded(TypedUUID uuid, size_t sizeBytes) override;
        /**
         * @brief Handles the budget exceeded callback.
         * @param currentUsage The current usage used by this operation.
         * @param budget The budget used by this operation.
         */
        void onBudgetExceeded(size_t currentUsage, size_t budget) override;

      private:
        /** @brief Stores the data for the asset info. */
        struct AssetInfo {
            /** @brief Stores the UUID. */
            TypedUUID uuid;
            /** @brief Stores the size in bytes. */
            size_t sizeBytes;
            /** @brief Stores the last access time. */
            uint64_t lastAccessTime;
        };

        /** @brief Stores the budget bytes. */
        size_t m_budgetBytes;
        /** @brief Stores the current usage. */
        size_t m_currentUsage = 0;
        /** @brief Stores the loaded assets. */
        std::vector<AssetInfo> m_loadedAssets;
    };

    // Priority-basierte Policy (z.B. für Streaming)
    /**
     * @brief Provides the priority memory policy functionality.
     * @tparam PriorityMemoryPolicy Template type used by this type.
     */
    class PriorityMemoryPolicy : public IMemoryBudgetPolicy {
      public:
        /**
         * @brief Creates an instance of priority memory policy.
         * @param budgetBytes The budget bytes used by this operation.
         */
        PriorityMemoryPolicy(size_t budgetBytes);

        /**
         * @brief Returns the current usage.
         * @return The result of the operation.
         */
        size_t getCurrentUsage() const override;
        /**
         * @brief Returns the budget.
         * @return The result of the operation.
         */
        size_t getBudget() const override;
        /**
         * @brief Selects assets eligible for eviction.
         * @param bytesNeeded The bytes needed used by this operation.
         * @return The result of the operation.
         */
        std::vector<TypedUUID> selectEvictionCandidates(size_t bytesNeeded) override;
        /**
         * @brief Handles the asset loaded callback.
         * @param uuid The UUID used by this operation.
         * @param sizeBytes The size bytes used by this operation.
         */
        void onAssetLoaded(TypedUUID uuid, size_t sizeBytes) override;
        /**
         * @brief Handles the asset unloaded callback.
         * @param uuid The UUID used by this operation.
         * @param sizeBytes The size bytes used by this operation.
         */
        void onAssetUnloaded(TypedUUID uuid, size_t sizeBytes) override;

        // Setze Priorität eines Assets (höher = später evicted)
        /**
         * @brief Sets asset priority.
         * @param uuid The UUID used by this operation.
         * @param priority The priority used by this operation.
         */
        void setAssetPriority(TypedUUID uuid, int priority);

      private:
        /** @brief Stores the data for the asset info. */
        struct AssetInfo {
            /** @brief Stores the UUID. */
            TypedUUID uuid;
            /** @brief Stores the size in bytes. */
            size_t sizeBytes;
            /** @brief Stores the priority. */
            int priority = 0;
        };

        /** @brief Stores the budget bytes. */
        size_t m_budgetBytes;
        /** @brief Stores the current usage. */
        size_t m_currentUsage = 0;
        /** @brief Stores the loaded assets. */
        std::vector<AssetInfo> m_loadedAssets;
    };

} // namespace axiom
