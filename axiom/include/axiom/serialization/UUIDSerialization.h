// UUIDSerialization.h
#pragma once
#include "axiom/core/TypedUUID.h"
#include <iomanip>
#include <nlohmann/json.hpp>
#include <sstream>

namespace nlohmann {
    template <typename Tag> struct adl_serializer<axiom::TaggedUUID<Tag>> {
        static void to_json(json &j, const axiom::TaggedUUID<Tag> &id) {
            std::ostringstream oss;
            oss << std::hex << std::setw(16) << std::setfill('0')
                << id.Raw().Value();
            j = oss.str();
        }
        static void from_json(const json &j, axiom::TaggedUUID<Tag> &id) {
            uint64_t raw = std::stoull(j.get<std::string>(), nullptr, 16);
            id = axiom::TaggedUUID<Tag>(axiom::UUID(raw));
        }
    };
} // namespace nlohmann