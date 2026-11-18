/**
 * @file axiom/include/axiom/renderer/RenderLayerRegistry.h
 * @brief Declares the public interface for the render layer registry.
 */

#pragma once
#include <cstdint>
#include <string>
#include <array>
#include <unordered_map>
#include <optional>

namespace axiom::renderer {

/**
 * @brief Provides the render layer mask type alias.
 */
using RenderLayerMask = uint32_t;

// Keine hartkodierten Layer (siehe Design-Doc Abschnitt 7). Layer-Namen
// werden im Editor definiert und auf Bits einer 32-bit-Bitmask gemappt.
// Die Bitmask bleibt zur Laufzeit performant, Namen/Zuordnung sind
// Projektdaten (serialisierbar, im Editor editierbar).
/**
 * @brief Provides the render layer registry functionality.
 * @tparam RenderLayerRegistry Template type used by this type.
 */
class RenderLayerRegistry {
public:
    /** @brief Stores the k max layers. */
    static constexpr uint8_t kMaxLayers = 32;
    /** @brief Stores the k invalid bit. */
    static constexpr uint8_t kInvalidBit = 0xFF;
    /** @brief Stores the k default layer name. */
    static constexpr const char* kDefaultLayerName = "Default";

    // Reserviert "Default" auf Bit 0 - Fallback fuer Entities ohne
    // RenderLayerComponent (siehe GeometrySubmissionSystem).
    /**
     * @brief Creates an instance of render layer registry.
     */
    RenderLayerRegistry();

    // Vergibt das naechste freie Bit (0-31) fuer diesen Namen. Bei bereits
    // registriertem Namen wird dessen bestehendes Bit zurueckgegeben (idempotent).
    /**
     * @brief Registers layer.
     * @param name The name used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] std::optional<uint8_t> registerLayer(const std::string& name);

    // Gibt das Bit wieder frei - bestehende RenderLayerMask-Werte mit diesem
    // Bit werden dadurch NICHT automatisch aktualisiert (Aufrufer-Verantwortung,
    // z.B. beim Loeschen eines Layers im Editor betroffene Komponenten anpassen).
    // "Default" (Bit 0) kann NICHT entfernt werden.
    /**
     * @brief Unregisters layer.
     * @param name The name used by this operation.
     */
    void unregisterLayer(const std::string& name);

    /**
     * @brief Performs the mask for operation.
     * @param name The name used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] RenderLayerMask maskFor(const std::string& name) const;
    /**
     * @brief Performs the bit for operation.
     * @param name The name used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] std::optional<uint8_t> bitFor(const std::string& name) const;
    /**
     * @brief Returns the name of.
     * @param bit The bit used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] const std::string& nameOf(uint8_t bit) const;
    /**
     * @brief Reports whether registered.
     * @param name The name used by this operation.
     * @return The result of the operation.
     */
    [[nodiscard]] bool isRegistered(const std::string& name) const;

    // --- Serialisierung (Projekt-Config) ---
    // Einfaches Format: "bit;name" pro Zeile. Editor-UI/Asset-Format kann das
    // spaeter durch etwas Strukturierteres (JSON/YAML) ersetzen - hier bewusst
    // minimal, um Phase 4 nicht an ein noch nicht existierendes Asset-Format
    // zu koppeln.
    /**
     * @brief Serializes the object.
     * @return The result of the operation.
     */
    [[nodiscard]] std::string serialize() const;
    /**
     * @brief Deserializes the object.
     * @param data The data used by this operation.
     */
    void deserialize(const std::string& data);

private:
    /** @brief Stores the names. */
    std::array<std::string, kMaxLayers> m_names;
    /** @brief Stores the used. */
    std::array<bool, kMaxLayers> m_used{};
    /** @brief Stores the name to bit. */
    std::unordered_map<std::string, uint8_t> m_nameToBit;

    /** @brief Stores the empty name. */
    static const std::string sEmptyName;
};

} // namespace axiom::renderer
