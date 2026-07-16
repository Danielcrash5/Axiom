#pragma once
#include <cstdint>
#include <string>
#include <array>
#include <unordered_map>
#include <optional>

namespace axiom::renderer {

using RenderLayerMask = uint32_t;

// Keine hartkodierten Layer (siehe Design-Doc Abschnitt 7). Layer-Namen
// werden im Editor definiert und auf Bits einer 32-bit-Bitmask gemappt.
// Die Bitmask bleibt zur Laufzeit performant, Namen/Zuordnung sind
// Projektdaten (serialisierbar, im Editor editierbar).
class RenderLayerRegistry {
public:
    static constexpr uint8_t kMaxLayers = 32;
    static constexpr uint8_t kInvalidBit = 0xFF;
    static constexpr const char* kDefaultLayerName = "Default";

    // Reserviert "Default" auf Bit 0 - Fallback fuer Entities ohne
    // RenderLayerComponent (siehe GeometrySubmissionSystem).
    RenderLayerRegistry();

    // Vergibt das naechste freie Bit (0-31) fuer diesen Namen. Bei bereits
    // registriertem Namen wird dessen bestehendes Bit zurueckgegeben (idempotent).
    [[nodiscard]] std::optional<uint8_t> registerLayer(const std::string& name);

    // Gibt das Bit wieder frei - bestehende RenderLayerMask-Werte mit diesem
    // Bit werden dadurch NICHT automatisch aktualisiert (Aufrufer-Verantwortung,
    // z.B. beim Loeschen eines Layers im Editor betroffene Komponenten anpassen).
    // "Default" (Bit 0) kann NICHT entfernt werden.
    void unregisterLayer(const std::string& name);

    [[nodiscard]] RenderLayerMask maskFor(const std::string& name) const;
    [[nodiscard]] std::optional<uint8_t> bitFor(const std::string& name) const;
    [[nodiscard]] const std::string& nameOf(uint8_t bit) const;
    [[nodiscard]] bool isRegistered(const std::string& name) const;

    // --- Serialisierung (Projekt-Config) ---
    // Einfaches Format: "bit;name" pro Zeile. Editor-UI/Asset-Format kann das
    // spaeter durch etwas Strukturierteres (JSON/YAML) ersetzen - hier bewusst
    // minimal, um Phase 4 nicht an ein noch nicht existierendes Asset-Format
    // zu koppeln.
    [[nodiscard]] std::string serialize() const;
    void deserialize(const std::string& data);

private:
    std::array<std::string, kMaxLayers> m_names;
    std::array<bool, kMaxLayers> m_used{};
    std::unordered_map<std::string, uint8_t> m_nameToBit;

    static const std::string s_emptyName;
};

} // namespace axiom::renderer
