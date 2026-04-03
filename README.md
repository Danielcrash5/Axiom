# Axiom  
Eine moderne Spiele-Engine mit Fokus auf Plattformunabhängigkeit.  

---

## 📚 Inhaltsverzeichnis
1. [Kernsysteme](#kernsystme)
2. [Phase 1 — Rendering‑Grundlage](#phase-1--rendering-grundlage)
3. [Phase 2 — Asset‑System](#phase-2--asset-system-inkl-asset-packs)
4. [Phase 3 — ECS](#phase-3--ecs-produktionsreif)
5. [Phase 4 — Renderer‑Integration](#phase-4--renderer-integration)
6. [Phase 5 — Debug‑Tools / Editor‑Basis](#phase-5--debug-tools--editor-basis)
7. [Phase 6 — 2D‑Platformer‑Prototyp](#phase-6--2d-platformer-prototyp)
8. [Phase 7 — Minimaler Editor](#phase-7--minimaler-editor)
9. [Phase 8 — Physik (Box2D)](#phase-8--physik-box2d)
10. [Phase 9 — Skripting (Lua + Sol2 + C++)](#phase-9--skripting-lua--sol2--c)
11. [Phase 10 — Audio (OpenAL)](#phase-10--audio-openal)
12. [Phase 11 — Finaler Platformer](#phase-11--finaler-platformer)
13. [Phase 12 — Netzwerk (Asio)](#phase-12--netzwerk-asio)
14. [Phase 13 — Multiplayer Dungeon Crawler (2D)](#phase-13--multiplayer-dungeon-crawler-2d)
15. [Phase 14 — Benutzeroberflächensystem](#phase-14--benutzeroberflächensystem)
16. [Phase 15 — Editor‑Erweiterung](#phase-15--editor-erweiterung)
17. [Phase 16 — Frame Graph / Command List](#phase-16--frame-graph--command-list)
18. [Phase 17 — 3D‑Renderer](#phase-17--3d-renderer)
19. [Phase 18 — 3D‑Physik (Jolt)](#phase-18--3d-physik-jolt)
20. [Phase 19 — Plugin‑System](#phase-19--plugin-system)
21. [Phase 20 — Spiel‑Export](#phase-20--spiel-export)
22. [Multithreading](#multithreading)

---

## Kernsysteme (bereits implementiert)
- [x] Anwendungssystem (Main Loop, Lebenszyklus, Init/Shutdown)  
- [x] Logger (Level, Ausgabe, Formatierung)  
- [x] Profiler (Scopes, Frame Timings)  
- [x] Fenstersystem (Kontext-Erstellung, Resize, Close Events)  
- [x] Eingabesystem (Tastatur, Maus, Joystick)  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

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
  ```cpp
  struct RenderState {
      bool DepthTest;
      bool Blending;
      bool CullFace;
      // optional: BlendMode, DepthFunc
  };
  ```  
- [x] RendererAPI: SetRenderState(RenderState state)  
- [x] OpenGL-Mapping:  
  - DepthTest → glEnable/glDisable(GL_DEPTH_TEST)  
  - Blending → glEnable/glDisable(GL_BLEND)  
  - CullFace → glEnable/glDisable(GL_CULL_FACE)  
- [x] State-Cache: aktuelle Zustände speichern, nur ändern wenn nötig  
- [x] Standardzustand beim Init setzen: Blending=true, DepthTest=false, CullFace=false  

- [x] Abhaken, wenn keine direkten glEnable/glDisable-Calls mehr im Code vorhanden sind  

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

### Pipeline-System
- [ ] PipelineSpecification: Shader + RenderState + BlendMode  
- [ ] Pipeline-Interface und OpenGL-Implementierung  
- [x] Erledigt, wenn Pipeline gebunden werden kann  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 2 — Asset-System (inkl. Asset Packs)
**Kern:**  
- [ ] AssetHandle (UUID)  
- [ ] Asset-Basisklasse  
- [ ] AssetType-Enum  

**Asset-Manager:**  
- [ ] LoadAsset(handle)  
- [ ] GetAsset<T>()  
- [ ] UnloadAsset()  

**Asset-Registry:**  
- [ ] JSON/YAML-Registry-Datei  
- [ ] Mapping: Handle → Dateipfad + Typ  

**Asset-Loader:**  
- [ ] Texture-Loader  
- [ ] Shader-Loader (Slang)  
- [ ] Audio-Loader  

**Asset-Lebenszyklus:**  
- [ ] Lazy Loading  
- [ ] Asset-Cache  
- [ ] Duplikatvermeidung  

**Asset Packs:**  
- [ ] AssetPack-Klasse  
- [ ] Mount-/Unmount-System  
- [ ] Mehrere Packs gleichzeitig laden  
- [ ] Override-System (Game > Engine)  
- [ ] Optional: Binäre Packs, Streaming  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 3 — ECS (Produktionsreif)
**Kern:**  
- [ ] Entity (ID + Version)  
- [ ] Komponenten-Speicher (kontiguierlich)  
- [ ] Add/Remove Component  

**Abfragesystem:**  
- [ ] View<T...>  
- [ ] Cache-freundliche Iteration  

**Komponenten:**  
- [ ] TransformComponent  
- [ ] SpriteRendererComponent  
- [ ] CameraComponent  

**Systeme:**  
- [ ] Transform-System  
- [ ] Render-System  

**Erweiterungen:**  
- [ ] Parent/Child-Hierarchie  
- [ ] Transform-Propagation  
- [ ] Dirty-Flags  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 4 — Renderer-Integration
- [ ] ECS → DrawCommandBuffer  
- [ ] DrawCommand-Struktur (Mesh + Material + Transform)  
- [ ] Sortierung (nach Material/Textur)  
- [ ] Batching  
- [ ] Kamera-Handling (View/Projection)  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 5 — Debug Tools / Editor-Basis
- [ ] ImGui-Integration  
- [ ] ImGuizmo-Integration  
- [ ] Panels: Hierarchie, Inspektor, Statistiken  
- [ ] Transform-Bearbeitung + Gizmos  
- [ ] Andockbare Benutzeroberfläche  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 6 — 2D-Platformer-Prototyp
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

## Phase 9 — Skripting (Lua + Sol2 + C++)
- [ ] Lua-Integration
- [ ] dynamisches laden von C++ Dlls
- [ ] Skript-Komponente für Lua und C++

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 10 — Audio (OpenAL)
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
- [ ] Entitätssynchronisierung  
- [ ] Basis-Replikation  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 13 — Multiplayer Dungeon Crawler (2D)
- [ ] Koop-Gameplay  
- [ ] Gegner/KI  
- [ ] Level-Design  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)

---

## Phase 14 — Benutzeroberflächensystem
- [ ] Spiel-HUD  
- [ ] Menüs  

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

## Multithreading
**Vorbereitung:**  
- [ ] Keine globalen Renderer-Zustände  
- [ ] Thread-sicheres CommandBuffer-Design  

**Implementierung:**  
- [ ] Job-System  
- [ ] Thread-Pool  
- [ ] Asynchrones Asset-Loading  
- [ ] Parallele ECS-Verarbeitung  
- [ ] Parallele Renderer-Kommandos  

[⬆ Zurück zum Anfang](#📚-inhaltsverzeichnis)
