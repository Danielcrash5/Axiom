#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace axiom::renderer {

    enum class ShaderStageKind { Vertex, Fragment, Compute };

    // Kompiliert GLSL (Vulkan-Dialekt: #version 450+, layout(set=,binding=))
    // zu SPIR-V, in-process ueber shaderc (Teil des Vulkan SDK) - kein
    // externer Tool-Aufruf, kein Download zur Buildzeit. Ersetzt
    // tools/bake_shaders.py.
    //
    // Bewusst eigenstaendig, ohne Editor- oder Packager-Wissen: beide
    // koennen dieselbe Klasse nutzen - der Editor fuer Live-Kompilierung
    // beim Speichern/Importieren, ein spaeterer Packager fuer den
    // Offline-Bake-Schritt beim Shippen. ShaderRegistry::compileFromSource()
    // ist der uebliche Einstiegspunkt fuer beide; diese Klasse direkt nur,
    // wenn man SPIR-V ohne Registry-Eintrag braucht (z.B. ein
    // Packager-Tool, das nur .spv-Dateien auf die Platte schreiben will).
    class ShaderCompiler {
      public:
        struct Result {
            std::vector<uint32_t> spirv;
            std::string errors;   // leer bei Erfolg, sonst Compiler-Diagnose
            std::string warnings; // kann auch bei Erfolg gefuellt sein
        };

        // source: GLSL-Quelltext (nicht Dateipfad - Aufrufer liest z.B. ueber
        // VFS::ReadFile, damit #include-Aufloesung ueber Mounts moeglich
        // bleibt und der Editor Live-Text ohne Zwischendatei kompilieren
        // kann). sourceName: nur fuer Fehlermeldungen (z.B.
        // "engine://shaders/Quad.vert").
        [[nodiscard]] Result Compile(const std::string &source,
                                     const std::string &sourceName,
                                     ShaderStageKind stage) const;
    };

} // namespace axiom::renderer
