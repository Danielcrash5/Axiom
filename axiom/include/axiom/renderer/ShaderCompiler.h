/**
 * @file axiom/include/axiom/renderer/ShaderCompiler.h
 * @brief Declares the public interface for the shader compiler.
 */

#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace axiom::renderer {

    /**
     * @brief Defines the available shader stage kind values.
     * @tparam ShaderStageKind Template type used by this type.
     */
    enum class ShaderStageKind { vertex, fragment, compute };

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
    /**
     * @brief Provides the shader compiler functionality.
     * @tparam ShaderCompiler Template type used by this type.
     */
    class ShaderCompiler {
      public:
        /** @brief Stores the data for the result. */
        struct Result {
            /** @brief Stores the spirv. */
            std::vector<uint32_t> spirv;
            /** @brief Stores the errors. */
            std::string errors;   // leer bei Erfolg, sonst Compiler-Diagnose
            /** @brief Stores the warnings. */
            std::string warnings; // kann auch bei Erfolg gefuellt sein
        };

        // source: GLSL-Quelltext (nicht Dateipfad - Aufrufer liest z.B. ueber
        // VFS::ReadFile, damit #include-Aufloesung ueber Mounts moeglich
        // bleibt und der Editor Live-Text ohne Zwischendatei kompilieren
        // kann). sourceName: nur fuer Fehlermeldungen (z.B.
        // "engine://shaders/Quad.vert").
        /**
         * @brief Compiles the shader source.
         * @param source The source used by this operation.
         * @param sourceName The source name used by this operation.
         * @param stage The stage used by this operation.
         * @return The result of the operation.
         */
        [[nodiscard]] Result compile(const std::string &source,
                                     const std::string &sourceName,
                                     ShaderStageKind stage) const;
    };

} // namespace axiom::renderer
