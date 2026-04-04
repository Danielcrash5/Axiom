#pragma once
#include "Asset.h"

namespace axiom {

    template<AssetType T>
    class TypedAsset : public Asset {
    public:
        static constexpr AssetType Type = T;

        virtual AssetType GetType() const override {
            return T;
        }
    };

}