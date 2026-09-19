# Axiom Renderer – Fixes & Swapchain/Present-Implementierung

## 1. Dateien in diesem ZIP direkt überschreiben
Alle Dateien hier haben denselben relativen Pfad wie in deinem Repo – einfach
drüberkopieren.

## 2. Diese Dateien musst du LÖSCHEN (Case-Mismatches, echte Ordner-Duplikate)
Ein ZIP kann keine Löschungen ausdrücken – das musst du manuell nachziehen:

```
axiom/include/axiom/ImGui/IImGuilayer.h        (ersetzt durch IImGuiLayer.h)
axiom/include/axiom/Input/ActionMap.h          (ersetzt durch input/ActionMap.h)
axiom/include/axiom/Input/GamepadCodes.h       (ersetzt durch input/GamepadCodes.h)
axiom/include/axiom/Input/Input.h              (ersetzt durch input/Input.h)
axiom/include/axiom/Input/KeyCodes.h           (ersetzt durch input/KeyCodes.h)
axiom/include/axiom/Input/MouseCodes.h         (ersetzt durch input/MouseCodes.h)
axiom/include/axiom/core/Layerstack.h          (ersetzt durch LayerStack.h)
axiom/src/Input/ActionMap.cpp                  (ersetzt durch src/input/ActionMap.cpp)
axiom/src/Input/Input.cpp                      (ersetzt durch src/input/Input.cpp)
axiom/src/core/Layerstack.cpp                  (ersetzt durch LayerStack.cpp)
axiom/include/axiom/renderer/rhi/Commandlist.h (ersetzt durch CommandList.h)
```

Danach solltest du KEINE zwei Ordner mehr haben, die sich nur in Groß-/
Kleinschreibung unterscheiden (`Input/` vs `input/` gab's tatsächlich beide
gleichzeitig im Repo – auf Windows/macOS unsichtbar, auf Linux zwei getrennte
Verzeichnisse).

## 3. VMA (Vulkan Memory Allocator) vendoren
`VulkanBackend.h`/`VmaImpl.cpp` brauchen `<vma/vk_mem_alloc.h>` – das gibt's
im Repo aktuell NIRGENDS (weder Submodule noch Fetch noch vendored Datei).
Lade die aktuelle Single-Header-Version:

```
https://raw.githubusercontent.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/master/include/vk_mem_alloc.h
```

und leg sie ab unter:
```
thirdparty/vma/vma/vk_mem_alloc.h
```
(die doppelte `vma/vma/`-Verschachtelung ist Absicht – der Include-Pfad
`thirdparty/vma` wird als Include-Directory gesetzt, damit `#include
<vma/vk_mem_alloc.h>` funktioniert.)

`axiom/CMakeLists.txt` (in diesem ZIP enthalten) fügt den Include-Pfad schon
für `axiom_engine` und `axiom_engine_test` hinzu.

## 4. Was inhaltlich neu ist

**Swapchain/Present (fehlte komplett):**
- `rhi/Swapchain.h` (neu): `SwapchainDesc`, `AcquiredImage`
- `Handle.h`: `SwapchainHandle` ergänzt
- `IRHIBackend.h`: `createSwapchain`/`destroySwapchain`/`acquireNextImage`/
  `present`/`swapchainFormat`/`swapchainExtent`
- `VulkanBackend.h`/`.cpp`: vollständige Implementierung – Capabilities/
  Format/Present-Mode-Auswahl, automatisches Resize bei
  `VK_ERROR_OUT_OF_DATE_KHR`, ein stabiler `TextureHandle` pro Swapchain-Image
  (kein Handle-Churn pro Frame), vollständig synchron (passt zum bisherigen
  Stil des Backends – kein Frames-in-Flight-Overlap, das ist eine spätere
  Optimierung)
- **Zwei Bugs im Zuge dessen gefixt:** Destruktor hätte mit Swapchain-Images
  abgestürzt (`vmaDestroyImage` auf einem Image ohne `VmaAllocation`), und
  es gab noch gar keine Stelle, die Swapchains zerstört (jetzt: vor
  Surfaces/Device, wie von Vulkan verlangt).

**`compile()` läuft nicht mehr jeden Frame bedingungslos neu (Perf-Fix):**
- Vergleicht jetzt Größe/Format/Usage jeder transienten Resource gegen den
  letzten `compile()`-Aufruf und nur bei tatsächlicher Änderung neu erzeugt.
- Test dafür umgeschrieben (`RecompileReusesUnchangedTransientTextures`
  ersetzt den alten `RecompileReleasesPreviousTransientTextures`-Test, der
  genau das alte, unerwünschte Verhalten erwartet hatte).

**Passes kennen jetzt die aktuelle View während `setup()`:**
- `RenderGraphBuilder` bekommt dieselbe `RenderExecutionDesc` wie
  `RenderContext` (inkl. `presentTarget()` fürs Swapchain-Image des aktuellen
  Frames) – `compile()` nimmt jetzt einen optionalen `RenderExecutionDesc`-
  Parameter.
- `ClearScreenPass` nutzt das schon: Größe kommt jetzt aus der aktuellen
  View statt hartkodiert 800×600 (Fallback bleibt 800×600, falls keine View
  übergeben wird, z.B. in isolierten Unit-Tests).

## 5. Verifiziert
Komplettes Projekt (Engine, alle 3 Test-Targets, testbed) baut fehlerfrei
mit GCC/Ninja auf Linux. Alle 3 Renderer-Tests laufen grün.

**Nicht verifiziert:** Läuft nie gegen eine echte GPU/Fenster (Sandbox ohne
Display/Vulkan-Treiber) – reine Compile+Logik-Verifikation über den
`NullBackend` in den Unit-Tests. Realer Swapchain-Betrieb (echtes Fenster,
echte Präsentation) solltest du selbst einmal durchklicken.
