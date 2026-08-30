#include <axiom/assets/TypedUUID.h>
#include <algorithm>
#include <random>
#include <sstream>

namespace axiom {

    std::string TypedUUID::ToString() const {
        std::stringstream ss;
        ss << std::hex << std::uppercase;
        ss << high << "-" << low << "-" << static_cast<uint32_t>(type);
        return ss.str();
    }

    TypedUUID TypedUUID::FromString(const std::string &str) {
        TypedUUID uuid;
        std::stringstream ss(str);
        char dash;

        ss >> std::hex >> uuid.high >> dash >> uuid.low >> dash;
        uint32_t typeInt = 0;
        ss >> typeInt;
        uuid.type = static_cast<AssetTypeId>(typeInt);

        return uuid;
    }

    TypedUUID TypedUUID::Generate(AssetTypeId type) {
        static std::random_device rd;
        static std::mt19937_64 gen(rd());
        static std::uniform_int_distribution<uint64_t> dis;

        return TypedUUID(dis(gen), dis(gen), type);
    }

} // namespace axiom
