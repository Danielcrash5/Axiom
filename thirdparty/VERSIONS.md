# Third-Party-Abhängigkeiten

Alle Bibliotheken unter `thirdparty/` sind als Quellcode kopiert (kein Package Manager, keine Submodule).
Diese Datei ist die einzige Quelle der Wahrheit für Herkunft, Version und lokale Änderungen.

**Regeln**

1. Jede Änderung unter `thirdparty/` geht nur zusammen mit einem Update dieser Datei in den Commit.
2. Lokale Patches werden hier aufgelistet und im Code mit `// AXIOM-PATCH: <Grund>` markiert,
   damit sie sich beim Update per `grep -rn "AXIOM-PATCH" thirdparty/` wiederfinden lassen.
3. Version = Release-Tag, falls vorhanden, sonst voller Commit-Hash (kein Branch-Name).
4. Lizenzen stehen hier nur als Kurzangabe. Maßgeblich ist die mitgelieferte LICENSE-Datei der jeweiligen Bibliothek.

## Aktuell eingebunden

> Alle Felder `TODO` ausfüllen. Ordnernamen mit `ls thirdparty/` abgleichen und Einträge für Bibliotheken
> löschen, die nicht (mehr) im Repo liegen.

| Ordner | Zweck | Upstream | Version (Tag/Commit) | Lizenz (kurz) | Eingebunden als | Lokale Patches |
|---|---|---|---|---|---|---|
| `flecs` | ECS | https://github.com/SanderMertens/flecs | TODO | MIT | Amalgamierte Quelle (`flecs.c`/`flecs.h`) | TODO (keine?) |
| `shaderc` | GLSL → SPIR-V zur Laufzeit | https://github.com/google/shaderc | TODO | Apache-2.0 | Quellcode, `add_subdirectory` | TODO |
| `glslang` | Abhängigkeit von shaderc | https://github.com/KhronosGroup/glslang | TODO (laut `DEPS` von shaderc) | gemischt (überwiegend BSD-3-Clause) | Quellcode, via shaderc | TODO |
| `SPIRV-Tools` | Abhängigkeit von shaderc | https://github.com/KhronosGroup/SPIRV-Tools | TODO (laut `DEPS` von shaderc) | Apache-2.0 | Quellcode, via shaderc | TODO |
| `SPIRV-Headers` | Abhängigkeit von shaderc | https://github.com/KhronosGroup/SPIRV-Headers | TODO (laut `DEPS` von shaderc) | MIT-ähnlich | Quellcode, via shaderc | TODO |
| `SDL3` | Fenster, Input, Audio-Ausgabe | https://github.com/libsdl-org/SDL | TODO | zlib | TODO (Quellcode / Binary) | TODO |
| `glm` | Mathematik | https://github.com/g-truc/glm | TODO | MIT | Header-only | TODO |
| `imgui` | Editor-UI | https://github.com/ocornut/imgui | TODO (Branch `docking`?) | MIT | Quellcode | TODO |
| `ImGuizmo` | Gizmos im Editor | https://github.com/CedricGuillemet/ImGuizmo | TODO | MIT | Quellcode (CMake-Einbindung offen) | TODO |
| `vk_mem_alloc` (VMA) | GPU-Speicherverwaltung | https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator | TODO | MIT | Header (`VMA_IMPLEMENTATION` in genau einer TU) | TODO |
| `nlohmann_json` | JSON-Serialisierung | https://github.com/nlohmann/json | TODO | MIT | Single-Header | TODO |
| `cereal` | Binäre Serialisierung | https://github.com/USCiLab/cereal | TODO | BSD-3-Clause | Header-only | TODO |
| `zstd` | Kompression (`.axpak`) | https://github.com/facebook/zstd | TODO | BSD-3-Clause (Dual mit GPLv2) | TODO | TODO |
| `xxhash` | Hashing (`.axpak`, xxHash64) | https://github.com/Cyan4973/xxHash | TODO | BSD-2-Clause (Library) | TODO | TODO |

Nicht in der Tabelle, aber zu prüfen: Vulkan-Header/-Loader (kommen aus dem Vulkan SDK bzw. dem System,
nicht aus `thirdparty/`), `stb_*`, Box2D, Jolt, FiberTaskingLib, Luau. Nur eintragen, wenn tatsächlich eingebunden.

## shaderc und seine Abhängigkeiten

Die Revisionen von glslang, SPIRV-Tools und SPIRV-Headers müssen zu der shaderc-Version passen.
Maßgeblich ist die Datei `DEPS` im shaderc-Tag, das hier eingetragen ist. Diese drei Bibliotheken nie einzeln
aktualisieren, sondern nur gemeinsam mit shaderc.

Besonderheit Toolchain: Die vorkompilierten `shaderc_combined(d).lib` des Vulkan SDK sind MSVC-ABI und mit MinGW/libstdc++
nicht linkbar. Deshalb aus Quellcode.

## flecs

Es wird die amalgamierte Quelle verwendet. Beim Update: Amalgamation aus dem **gleichen Release-Tag** erzeugen,
den Tag oben eintragen und die API-Änderungen im Release-Changelog prüfen (flecs ändert zwischen Versionen die API).

## Update-Ablauf

1. Changelog der Bibliothek lesen. Auf Breaking Changes und Lizenzänderungen achten.
2. Alten Ordner durch den neuen Stand ersetzen (Dateien löschen, die es upstream nicht mehr gibt).
3. Lokale Patches per `grep -rn "AXIOM-PATCH" thirdparty/<lib>` im alten Stand finden und neu anwenden oder streichen.
4. Version, Lizenz und Patches in dieser Tabelle aktualisieren.
5. Komplett bauen (Debug und Release), Tests laufen lassen, erst dann committen.
6. Ein Commit pro Bibliothek, Message: `thirdparty: <lib> <alte Version> -> <neue Version>`.

## Entfernt

| Bibliothek | Grund | Datum |
|---|---|---|
| EnTT | Ersetzt durch flecs | TODO |
| WAMR (WebAssembly) | WASM-Scripting verworfen, Luau + native Plugins | TODO |
