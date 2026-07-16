#include <axiom/renderer/RenderLayerRegistry.h>
#include <sstream>

namespace axiom::renderer {

const std::string RenderLayerRegistry::s_emptyName;

RenderLayerRegistry::RenderLayerRegistry() {
    m_used[0] = true;
    m_names[0] = kDefaultLayerName;
    m_nameToBit[kDefaultLayerName] = 0;
}

std::optional<uint8_t> RenderLayerRegistry::registerLayer(const std::string& name) {
    if (auto it = m_nameToBit.find(name); it != m_nameToBit.end()) {
        return it->second; // idempotent - bereits registrierter Name gibt sein Bit zurueck
    }

    for (uint8_t bit = 0; bit < kMaxLayers; ++bit) {
        if (!m_used[bit]) {
            m_used[bit] = true;
            m_names[bit] = name;
            m_nameToBit[name] = bit;
            return bit;
        }
    }
    return std::nullopt; // alle 32 Bits belegt
}

void RenderLayerRegistry::unregisterLayer(const std::string& name) {
    if (name == kDefaultLayerName) return; // Bit 0 ist geschuetzt

    auto it = m_nameToBit.find(name);
    if (it == m_nameToBit.end()) return;

    uint8_t bit = it->second;
    m_used[bit] = false;
    m_names[bit].clear();
    m_nameToBit.erase(it);
}

RenderLayerMask RenderLayerRegistry::maskFor(const std::string& name) const {
    auto it = m_nameToBit.find(name);
    if (it == m_nameToBit.end()) return 0;
    return RenderLayerMask{1} << it->second;
}

std::optional<uint8_t> RenderLayerRegistry::bitFor(const std::string& name) const {
    auto it = m_nameToBit.find(name);
    if (it == m_nameToBit.end()) return std::nullopt;
    return it->second;
}

const std::string& RenderLayerRegistry::nameOf(uint8_t bit) const {
    if (bit >= kMaxLayers || !m_used[bit]) return s_emptyName;
    return m_names[bit];
}

bool RenderLayerRegistry::isRegistered(const std::string& name) const {
    return m_nameToBit.contains(name);
}

std::string RenderLayerRegistry::serialize() const {
    std::ostringstream out;
    for (uint8_t bit = 0; bit < kMaxLayers; ++bit) {
        if (m_used[bit]) {
            out << static_cast<int>(bit) << ";" << m_names[bit] << "\n";
        }
    }
    return out.str();
}

void RenderLayerRegistry::deserialize(const std::string& data) {
    m_names = {};
    m_used = {};
    m_nameToBit.clear();

    std::istringstream in(data);
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto separatorPos = line.find(';');
        if (separatorPos == std::string::npos) continue;

        int bit = std::stoi(line.substr(0, separatorPos));
        std::string name = line.substr(separatorPos + 1);
        if (bit < 0 || bit >= kMaxLayers || name.empty()) continue;

        m_used[static_cast<uint8_t>(bit)] = true;
        m_names[static_cast<uint8_t>(bit)] = name;
        m_nameToBit[name] = static_cast<uint8_t>(bit);
    }

    // Default-Layer ist geschuetzt und muss nach dem Laden garantiert existieren -
    // falls die geladenen Daten das nicht abdecken (z.B. altes/kaputtes Format),
    // hier erzwingen statt eine Registry ohne Fallback-Layer zu riskieren.
    if (!m_used[0]) {
        m_used[0] = true;
        m_names[0] = kDefaultLayerName;
        m_nameToBit[kDefaultLayerName] = 0;
    }
}

} // namespace axiom::renderer
