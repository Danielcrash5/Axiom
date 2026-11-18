/**
 * @file axiom/include/axiom/assets/Asset.h
 * @brief Declares the public interface for the asset.
 */

#pragma once

#include "TypedUUID.h"
#include "AssetType.h"

namespace axiom {
    /**
     * @brief Provides the asset functionality.
     * @tparam Asset Template type used by this type.
     */
    class Asset {
      public:
        /**
         * @brief Releases resources owned by Asset.
         */
        virtual ~Asset() = default;

        /**
         * @brief Performs the get ID operation.
         * @return The result of the operation.
         */
        TypedUUID GetID() const { return m_ID; }

        /**
         * @brief Performs the get type operation.
         * @return The result of the operation.
         */
        AssetType GetType() const { return m_Type; }

      protected:
        /** @brief Stores the ID. */
        TypedUUID m_ID{};
        /** @brief Stores the type. */
        AssetType m_Type = AssetType::Unknown;
    };
} // namespace axiom