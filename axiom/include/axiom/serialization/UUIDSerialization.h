// UUIDSerialization.h
#pragma once
#include "axiom/core/TaggedUUID.h"
#include <iomanip>
#include <nlohmann/json.hpp>
#include <sstream>

namespace nlohmann {
    // Voller 128-Bit-Wert (Hi+Lo), nicht nur die untere Haelfte - die alte
    // Fassung serialisierte nur einen uint64_t und rief zudem eine nicht
    // existierende UUID::Value()/UUID(uint64_t) auf, war also nie
    // kompilierbar (und wurde deshalb auch nirgends inkludiert).
    template <typename Tag> struct adl_serializer<axiom::TaggedUUID<Tag>> {
        static void to_json(json &j, const axiom::TaggedUUID<Tag> &id) {
            j = id.Raw().ToHexString();
        }
        static void from_json(const json &j, axiom::TaggedUUID<Tag> &id) {
            id = axiom::TaggedUUID<Tag>(axiom::UUID(j.get<std::string>()));
        }
    };
} // namespace nlohmann