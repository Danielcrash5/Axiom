#pragma once
#include "AssetHandle.h"
#include "AssetType.h"

namespace axiom {

    class Asset {
    public:
        virtual ~Asset() = default;

        AssetHandle GetHandle() const {
            return m_Handle;
        }
        void SetHandle(AssetHandle handle) {
            m_Handle = handle;
        }

        virtual AssetType GetType() const = 0;

    protected:
        AssetHandle m_Handle = InvalidAssetHandle;
    };

}