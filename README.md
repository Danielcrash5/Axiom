# Axiom  
Eine moderne Spiele-Engine mit Fokus auf Plattformunabhängigkeit.  

---

## 📚 Inhaltsverzeichnis
1. [Kernsysteme](#kernsystme)
2. [Phase 1 — Rendering‑Grundlage](#phase-1--rendering-grundlage)
3. [Phase 2 — Asset‑System](#phase-2--asset-system-inkl-asset-packs)
4. [Phase 3 — ECS](#phase-3--ecs-mithilfe-von-entt)
5. [Phase 4 — Renderer‑Integration](#phase-4--renderer-integration)
6. [Phase 5 — Debug‑Tools / Editor‑Basis](#phase-5--debug-tools--editor-basis)
7. [Phase 6 — 2D‑Platformer‑Prototyp](#phase-6--2d-platformer-prototyp)
8. [Phase 7 — Minimaler Editor](#phase-7--minimaler-editor)
9. [Phase 8 — Physik (Box2D)](#phase-8--physik-box2d)
10. [Phase 9 — Skripting (Luau + C++)](#phase-9--skripting-luau--c)
11. [Phase 10 — Audio (OpenAL)](#phase-10--audio-openal)
12. [Phase 11 — Finaler Platformer](#phase-11--finaler-platformer)
13. [Phase 12 — Netzwerk (Asio)](#phase-12--netzwerk-asio)
14. [Phase 13 — Benutzeroberflächensystem](#phase-13--benutzeroberflächensystem)
15. [Phase 14 — Multiplayer Dungeon Crawler (2D)](#phase-14--multiplayer-dungeon-crawler-2d)
16. [Phase 15 — Editor‑Erweiterung](#phase-15--editor-erweiterung)
17. [Phase 16 — Frame Graph / Command List](#phase-16--frame-graph--command-list)
18. [Phase 17 — 3D‑Renderer](#phase-17--3d-renderer)
19. [Phase 18 — 3D‑Physik (Jolt)](#phase-18--3d-physik-jolt)
20. [Phase 19 — Plugin‑System](#phase-19--plugin-system)
21. [Phase 20 — Spiel‑Export](#phase-20--spiel-export)
22. [Reflection](#reflection)
23. [Weitere Platformen](#weitere-platformen)
24. [Multithreading](#multithreading)

---

> **Hinweis zur Arbeitsreihenfolge:** Phase 9 (Skripting) wird **vor** Phase 7 (Minimaler Editor) bearbeitet, weil der Editor für Tool-Scripting eine eigene Luau-VM bekommt und den Luau-Wrapper daher als Abhängigkeit braucht. Die Nummerierung bleibt unverändert. Davor steht weiterhin der erste sichtbare Entity-Render-Pfad (Phase 4).

## Kernsysteme (bereits implementiert)
- [x] Anwendungssystem (Main Loop, Lebenszyklus, Init/Shutdown)  
- [x] Logger (Level, Ausgabe, Formatierung)  
- [x] Profiler (Scopes, Frame Timings)  
- [x] Fenstersystem (Kontext-Erstellung, Resize, Close Events)  
- [x] Eingabesystem (Tastatur, Maus, Joystick)  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Windows-Setup

WinLibs (MinGW-w64 UCRT GCC) muss installiert und dessen `mingw64\bin`-Verzeichnis
in `PATH` eingetragen sein. Das Verzeichnis muss `cmake`, `ninja`, `gcc` und `g++`
bereitstellen. Das Setup-Skript merkt den WinLibs-Pfad für die Presets, installiert
bei Bedarf Python und das Vulkan-SDK über winget und prüft die erforderlichen
Werkzeuge:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\install-windows-dependencies.ps1
```

Danach ein neues Terminal öffnen und konfigurieren und bauen:

```powershell
& "$env:AXIOM_WINLIBS_BIN\cmake.exe" --preset x64-Debug
& "$env:AXIOM_WINLIBS_BIN\cmake.exe" --build --preset x64-Debug-Build
```

Für einen Release-Build `x64-Release` und `x64-Release-Build` verwenden.

---

## Phase 1 — Rendering-Grundlage

### Renderer-Architektur
- [x] RendererAPI-Interface vorhanden  
- [x] RenderCommand-Wrapper existiert  
- [x] OpenGL-Backend angebunden  
- [x] Keine direkten OpenGL-Calls außerhalb des Backends  
- [x] Renderer wird zentral initialisiert (Renderer::Init())  
- [x] API-Auswahl vorbereitet (RendererAPIType Enum: OpenGL, Vulkan, etc.)  

### OpenGL-Backend (4.6)
- [x] Init()  
- [x] SetViewport()  
- [x] SetClearColor()  
- [x] Clear()  
- [x] DrawIndexed()  
- [x] Depth Test vorbereitet (glEnable(GL_DEPTH_TEST))  
- [x] OpenGL Debug Callback aktiviert  
- [x] Fehler-Logging  

### Render State Handling
**Ziele:**  
- Alle GPU-Zustände zentral verwalten  
- Keine redundanten OpenGL-Calls  
- Standardzustände definieren  

**Aufgaben:**  
- [x] RenderState-Struktur implementieren   
- [x] RendererAPI: SetRenderState(RenderState state)  
- [x] OpenGL-Mapping:  
  - DepthTest → glEnable/glDisable(GL_DEPTH_TEST)  
  - Blending → glEnable/glDisable(GL_BLEND)  
  - CullFace → glEnable/glDisable(GL_CULL_FACE)  
- [x] State-Cache: aktuelle Zustände speichern, nur ändern wenn nötig  
- [x] Standardzustand beim Init setzen: Blending=true, DepthTest=false, CullFace=false  

### Buffer-System
- [x] VertexBuffer (abstrakt)  
- [x] OpenGLVertexBuffer  
- [x] IndexBuffer (abstrakt)  
- [x] OpenGLIndexBuffer  
- [x] Korrekte Destruktoren, keine Speicherlecks  
- [x] Saubere Bind/Unbind-Implementierung  

### Vertex-Layout-System
- [x] ShaderDataType Enum (Float, Vec2, Vec3, Vec4, Mat3, Mat4, Int, Bool usw.)  
- [x] BufferElement-Struktur (Name, Typ, Größe, Offset, Normalized)  
- [x] VertexBufferLayout berechnet automatisch Stride/Offset  
- [x] Flexibel für 2D und 3D  
- [x] Erledigt, wenn beliebige Vertex-Strukturen korrekt an Shader übergeben werden  

### VertexArray (VAO)
- [x] Interface existiert  
- [x] OpenGLVertexArray implementiert  
- [x] AddVertexBuffer() funktioniert  
- [x] SetIndexBuffer() funktioniert  
- [x] OpenGL-Bindings korrekt (glEnableVertexAttribArray, glVertexAttribPointer)  
- [x] Erledigt, wenn VertexLayout im Shader korrekt ankommt  

### Shader-System
- [x] Interface (Bind(), Unbind())  
- [x] Fehler-Logging  
- [x] Laufzeit-Uniform- und Textur-Binding   
- [x] Shader-Kompilierung zur Laufzeit: GLSL → SPIR-V über vendortes shaderc (`ShaderCompiler`, `ShaderRegistry::compileFromSource`)  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 2 — Asset-System (inkl. Asset Packs)
Abgeschlossen

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 3 — ECS mithilfe von EnTT
[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 4 — Renderer-Integration
- [x] `RenderItem`-Struktur (Material + Vertex-/Index-Buffer + Transform)  
- [x] Sortierung (nach Material/Textur) und Batching (`RenderQueue`, `BatchBuilder`)  
- [x] RenderGraph: Present-Target wird pro Frame einmal zentral importiert, das Layout hängt am Zugriff, ein zweiter Schreiber auf ein geteiltes Ziel lädt statt zu clearen  
- [ ] ECS → RenderQueue: `Render2DSystem` ist Totcode (alte Renderer2D-/DrawCommandBuffer-API, referenziert nicht mehr existierende Komponenten), Neubau nötig  
- [ ] Erster sichtbarer Quad: Vulkan-GLSL-Shader, Einheits-Quad-Mesh, Entity-`RenderPass`  
- [ ] Kamera-Handling: `CameraComponent` existiert, muss noch in die View-Matrizen verdrahtet werden  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 5 — Debug Tools / Editor-Basis
- [x] ImGui-Integration (Vulkan- + SDL3-Backend, Multi-Viewport, VSync zur Laufzeit umschaltbar)  
- [x] Vulkan-Validation-Meldungen laufen über den Engine-Logger (mit VUID)  
- [ ] ImGuizmo-Integration  (Erst mal auf Eis gelegt; `add_subdirectory` und Linking in der CMake fehlen noch)
- [x] Panels: Hierarchie, Inspektor, Statistiken  
- [x] Transform-Bearbeitung + Gizmos (leider ohne Gizmos, nur über Inspector numerische Eingabe)
- [x] Andockbare Benutzeroberfläche  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 6 — 2D-Platformer-Prototyp 
Nur finaler Platformer
- [ ] Spielerbewegung  
- [ ] Sprungmechanik  
- [ ] Kollision (AABB)  
- [ ] Kamera folgt dem Spieler  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 7 — Minimaler Editor
- [ ] Szenen-Serialisierung/-Deserialisierung  
- [ ] Entitäts-/Komponentenbearbeitung  
- [ ] Play/Stop-Modus  
- [ ] Eigene Luau-VM im Editor für Tool-Scripting (eigener State, ohne Threading-Library; setzt Phase 9 voraus)  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 8 — Physik (Box2D)
- [ ] Physik-Welt  
- [ ] Rigidbody2D-Komponente  
- [ ] Collider2D  
- [ ] Kollisionsevents  
- [ ] Transform-Synchronisierung  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 9 — Skripting (Luau + C++)
> Wird vor Phase 7 bearbeitet (siehe Hinweis oben).

**Luau-Wrapper** (generische, von Axiom unabhängige Bibliothek, Design-Dokument: `LuauWrapper_Design.md`):  
- [ ] Kern-Wrapper (State, Table, Function, Userdata, Fehlerbehandlung, Compiler/Bytecode, Module)  
- [ ] Reflection-Integration (Reflection → Luau-Bindings) und `.d.luau`-Typdefinitionen  
- [ ] `SharedTable` + `SharedTableRegistry` als eigene Library (trennbar von `Actor`/`Future` über `openlibraries`)  
- [ ] Actor, Future, Scheduler (nach der Reflection-Integration, der Editor braucht sie nicht)  

**In der Engine:**  
- [ ] Skript-Komponente für Luau und C++  
- [ ] Dynamisches Laden von C++-Gameplay-/Plugin-DLLs (DLL entladen und neu laden) mit State-Erhaltung über den Reload: nur Werte, keine Pointer; Typ-ID = Hash aus Plugin-Name + Typname (Assert bei Kollision, Prüfung auf doppelte Plugin-Namen); Callbacks (EventBus, ImGuiPanelManager, Renderer) pro Plugin-/Luau-State-ID getaggt und vor dem Entladen entfernt  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 10 — Audio (SDL3)
- [ ] Audio-Gerät  
- [ ] Soundquelle  
- [ ] Wiedergabe  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 11 — Finaler Platformer
- [ ] Vollständiges Level mit Gameplay  
- [ ] Physik + Audio integriert  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 12 — Netzwerk (Asio)
- [ ] Client/Server-Architektur  
- [ ] UPnP-Unterstützung
- [ ] Entitätssynchronisierung  
- [ ] Basis-Replikation  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 13 — Benutzeroberflächensystem mit RmlUI
- [ ] Spiel-HUD  
- [ ] Menüs  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 14 — Multiplayer Dungeon Crawler (2D)
- [ ] Koop-Gameplay  
- [ ] Gegner/KI  
- [ ] Level-Design  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 15 — Editor-Erweiterung
- [ ] Asset-Browser  
- [ ] Prefabs  
- [ ] Undo/Redo  
- [ ] Erweiterte Szenenwerkzeuge  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 16 — Frame Graph / Command List
- [ ] Render-Pass-System  
- [ ] Ressourcenabhängigkeiten  
- [ ] Command-Graph-Ausführung  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 17 — 3D-Renderer
- [ ] Mesh-Rendering  
- [ ] Tiefentest  
- [ ] 3D-Kamera  
- [ ] Beleuchtung  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 18 — 3D-Physik (Jolt)
- [ ] Rigidbody3D  
- [ ] Collider3D  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 19 — Plugin-System
- [ ] Dynamische Module / Erweiterbarkeit  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 20 — Spiel-Export
- [ ] Build-Pipeline  
- [ ] Spiel-Paketierung  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Reflection
- [ ] Reflection für C++ Klassen (einfachere Einbindung in Luau und Inspector im Editor, Klassen/Systeme aus Gamecode-/Plugin-DLLs in der Engine sichtbar machen, State-Erhaltung über den DLL-Reload)
- [ ] Umsetzung über C++26-Reflection (P2996) mit Annotationen (P3394), GCC 16.1: `-std=c++26 -freflection`. In einem Testprojekt verifiziert: Namen, Annotationen, selektive Exponierung einzelner Member/Funktionen, Splice zu Pointer-to-Member, Vererbung (`bases_of`), Zugriffsrechte (`access_context`). Noch nicht ins Haupt-Repo übernommen
- [ ] RTTR ähnliche builder API (Alternative zu C++26-Reflection, falls diese nicht reicht)
- [ ] Custom-Attribut-System über Annotationen (Regel: keine Zeiger in Annotation-Structs, Text als `char[N]` by value)
- [ ] Laufzeit-Registry für die zur Compile-Zeit erzeugten Typbeschreibungen (pro DLL beim Laden eintragen, vor `FreeLibrary` austragen, keine STL-Container über die DLL-Grenze)
- [ ] auch nutzbar in Plugins und Modifikationen (auch wenn Mods hauptsächlich in Luau sind)

---

## Weitere Platformen
- [ ] Windows
- [ ] Web
- [ ] Linux
- [ ] Android
- [ ] VR
     
---

## Multithreading
> Aktuell zurückgestellt: kein konkreter Treiber (Bottleneck oder Feature). Ein einfacher `WorkerPool` existiert bereits als Interimslösung für asynchrones Asset-Laden im `AssetManager` (soll später durch das Fiber-Job-System ersetzt werden). Luaus Actor-/Scheduler-Modell (Phase 9) ist die wahrscheinlichere Heimat für weitere Concurrency-Primitive; erst angehen, wenn ein echter Bedarf besteht.

**Vorbereitung:**  
- [ ] Keine globalen Renderer-Zustände  
- [ ] Thread-sicheres CommandBuffer-Design  

**Implementierung:**  
- [ ] Job-System  
- [ ] Thread-Pool  
- [ ] Asynchrones Asset-Loading  
- [ ] Parallele ECS-Verarbeitung  
- [ ] Parallele Renderer-Kommandos
- [ ] Fibers

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)
