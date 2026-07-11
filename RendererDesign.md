# Axiom Renderer – Design Document

Referenzdokument für den Renderer der Axiom Game Engine. Wird laufend erweitert.

---

## 1. Grundprinzipien

- **Eine Renderer-Klasse.** Kein `Renderer2D` / `Renderer3D`-Split. 2D und 3D laufen über denselben Submission-Pfad, dieselbe Pipeline-Infrastruktur, dasselbe Material-System. Unterschiede (Vertexlayout, PipelineState, Shader) sind Daten, keine Klassenhierarchie.
- **Modular / nicht hartcodiert.** Die Standard-Pipeline ist ein austauschbares Setup aus Passes, kein fest verdrahteter Ablauf. Nutzer können Passes hinzufügen, entfernen, ersetzen.
- **CommandList-basiert.** Alle GPU-Arbeit läuft über ein CommandList-Interface, das Backends (Vulkan zuerst, WebGPU später nur für Web-Export-Target) implementieren.
- **3D wird nie blockiert.** Jede Design-Entscheidung für 2D darf 3D-Anwendungsfälle nicht ausschließen.
- **Custom Vertexformate.** Nutzer können eigene Vertex-Layouts (inkl. 4D+ Attribute) definieren und an die GPU senden, ohne Core-Code anzufassen.

---

## 2. Architektur-Überblick

**Alles ist ein Pass.** 2D, UI, 3D-Opaque, 3D-Transparent, Post-FX – es gibt keine separaten Subsysteme dafür, sondern jeweils eine `RenderPass`-Implementierung im `RenderGraph`. Das Default-Setup reiht sie in sinnvoller Reihenfolge, aber Nutzer können eigene Passes (z.B. eine komplett andere 3D-Pipeline) einschieben, ersetzen oder weglassen.

**Der RenderGraph ist reine Scheduling-Infrastruktur.** Er kennt nur Passes und deren Resource-Reads/-Writes (für Dependency-Auflösung, Barriers, Aliasing) – er weiß nichts von `RenderItem`s, Materialien oder Sortierung. Das ist bewusst getrennt: Sortierung und Batching leben in eigenen, vom Graph unabhängigen Subsystemen (`RenderQueueSystem`, `BatchBuilder`), die ein Pass in seiner `execute()`-Implementierung nutzt, die aber selbst kein Graph-Konzept sind.

```
ECS (Components)
      │
      ▼
RenderSubmissionSystem
      │  Liest Transform/Geometrie/MaterialComponent/RenderLayerComponent
      │  und baut daraus RenderItems – KEIN Wissen über Passes/Sortierung.
      ▼
Renderer::submitItem(item)  ──► globaler Item-Pool (unsortiert, mit Layer-Tag)
      │
      ▼
RenderQueueSystem  (eigenständig, NICHT Teil des RenderGraph)
      │  Filtert Item-Pool pro Pass nach RenderQueueDescriptor.acceptedLayers,
      │  sortiert via RenderSort::sort(items, descriptor.sortMode).
      ▼
RenderGraph.compile()
      │  Rein Resource-/Pass-Scheduling: Dependency-Graph, Barriers,
      │  Transient-Resource-Aliasing. Kennt keine RenderItems.
      ▼
RenderGraph.execute()
      │  Für jeden Pass: execute(ctx, cmd, sortedItems)
      │  Innerhalb des Passes: BatchBuilder::build(sortedItems)
      │  fasst gleiche Material/Bindless-Index zu instanced Draws zusammen.
      ▼
IRHIBackend (Vulkan / später WebGPU für Web) ──► GPU
```

---

## 3. RenderGraph & Passes

Pass-basierte Pipeline, keine feste Sequenz. Der Renderer besitzt einen `RenderGraph`, der zur Laufzeit aus Passes zusammengesetzt wird (Default-Setup vorhanden, aber komplett ersetzbar).

**Wichtige Abgrenzung:** Der `RenderGraph` ist ausschließlich für **Pass-Scheduling und Resource-Dependencies** zuständig – analog zu Frostbites Frame Graph oder Unreals RDG. Er weiß, dass "Pass A in Ressource X schreibt, Pass B aus X liest", und daraus leitet er Ausführungsreihenfolge, Barriers und Transient-Speicher-Aliasing ab. Er weiß **nichts** über `RenderItem`s, Materialien, Sortierung oder Batching – das sind Aufgaben der `execute()`-Implementierung eines Passes bzw. der Subsysteme, die dieser dort aufruft (siehe Abschnitt 10).

```cpp
class RenderPass {
public:
    virtual ~RenderPass() = default;
    virtual void setup(RenderGraphBuilder& builder) = 0;   // deklariert Resource-Reads/-Writes
    virtual void execute(RenderContext& ctx, CommandList& cmd) = 0; // eigentliche Zeichenarbeit
};

class RenderGraph {
    std::vector<std::unique_ptr<RenderPass>> passes;
public:
    void addPass(std::unique_ptr<RenderPass> pass);
    void compile();   // NUR Dependency-Auflösung, Barriers, Aliasing – keine Szenen-Logik
    void execute(RenderContext& ctx);
};
```

Ressourcen werden über virtuelle Handles referenziert (nicht über konkrete Texture/Buffer-Objekte), damit der Graph Aliasing/Transient-Speicher übernehmen kann und Passes voneinander entkoppelt bleiben.

**Nutzung von `RenderQueueSystem`/`BatchBuilder` ist optional, nicht vorgeschrieben.** Ein eigener Pass (z.B. ein Custom-PostFX-Pass für einen Schild-Effekt) kann diese Subsysteme nutzen, wenn er item-basierte, sortierbare Geometrie zeichnet – oder komplett eigene Logik in `execute()` schreiben (z.B. ein Fullscreen-Pass über eine Masken-Textur, ganz ohne `RenderItem`s). Der Graph unterscheidet nicht zwischen "Standard"- und "Custom"-Passes; beide sind einfach `RenderPass`-Implementierungen.

**Default-Passes (austauschbar, keine Sonderfälle im Renderer-Core):**

| Pass | Zweck |
|---|---|
| `Pass2D` | Sprites, Quads, Kreise, Linien/Linestrips, 2D-Skinned-Meshes |
| `PassUI` | UI-Elemente, immer im Vordergrund, eigene Queue/Sortierung |
| `Pass3DOpaque` | Undurchsichtige 3D-Geometrie |
| `Pass3DTransparent` | Transparente 3D-Geometrie |
| `PostEffectPass` | Layer-maskierte Post-Effekte (siehe Abschnitt 11) |

Jeder dieser Passes ist eine ganz normale `RenderPass`-Implementierung – der Renderer-Core kennt sie nicht namentlich, sie werden nur im Default-Pipeline-Setup registriert.

---

## 4. RHI-Abstraktion (Backend-Layer)

Erstes Backend: **Vulkan** (Desktop). **WebGPU** (Dawn) wird zusätzlich unterstützt, aber ausschließlich als Backend für ein **Web-Export-Target** (Browser via WASM) – kein primärer Desktop-Pfad mehr.

```cpp
class CommandList {
public:
    virtual void bindPipeline(PipelineHandle) = 0;
    virtual void bindVertexBuffer(uint32_t slot, BufferHandle) = 0;
    virtual void bindIndexBuffer(BufferHandle, IndexFormat) = 0;
    virtual void bindGroup(uint32_t set, BindGroupHandle) = 0;
    virtual void draw(uint32_t vtxCount, uint32_t instCount, uint32_t firstVtx, uint32_t firstInst) = 0;
    virtual void drawIndexed(uint32_t idxCount, uint32_t instCount, uint32_t firstIdx) = 0;
    virtual void dispatch(uint32_t x, uint32_t y, uint32_t z) = 0;
    virtual void copyBuffer(BufferHandle src, BufferHandle dst, ...) = 0;
};

class IRHIBackend {
public:
    virtual std::unique_ptr<CommandList> createCommandList() = 0;
    virtual void submit(CommandList&) = 0;
    virtual PipelineHandle createPipeline(const PipelineDesc&) = 0;
    virtual BufferHandle createBuffer(const BufferDesc&) = 0;
    virtual TextureHandle createTexture(const TextureDesc&) = 0;
    // ...
};
```

Shader-Typen: **Vertex, Pixel (Fragment), Compute** (Bitmask `ShaderStageMask`).

---

## 5. Vertexformate (datengetrieben, kein Struct-Zwang)

```cpp
enum class VertexFormat { Float32, Float32x2, Float32x3, Float32x4, Uint32, Uint8x4Norm };

struct VertexAttribute {
    std::string semantic;   // z.B. "POSITION", "TEXCOORD0", "CUSTOM_W" für 4D+
    uint32_t location;
    VertexFormat format;
    uint32_t offset;
};

struct VertexLayout {
    uint32_t stride;
    std::vector<VertexAttribute> attributes;
};
```

Pipelines werden aus `VertexLayout + Shader` gebaut. Nutzer können beliebige eigene Layouts definieren (z.B. für 4D-Geometrie oder exotische Vertex-Daten), ohne dass der Renderer-Core diese kennen muss.

---

## 6. Material

Ein einziges `Material` für 2D und 3D. Das Material **besitzt die ShaderID**.

```cpp
struct ShaderDesc {
    ShaderStageMask stages;
    ShaderID id;
    std::string vertexSrc, pixelSrc, computeSrc; // oder WGSL/SPIR-V Bytecode
    VertexLayout vertexLayout; // nur relevant falls Vertex-Stage vorhanden
};

class Material {
public:
    ShaderID shaderId;
    std::unordered_map<std::string, ResourceBinding> params; // Texturen, Buffer, Uniforms
    PipelineState state; // Blend, DepthTest, Cull, Topology
};
```

Ein Sprite mit eigenem Shader = eigene `Material`-Instanz mit anderer `ShaderID`. 3D-Meshes nutzen dieselbe `Material`-Klasse mit anderem Layout/PipelineState (z.B. DepthTest an, andere Topology).

`ResourceBinding` kann entweder ein direktes Binding (klassisches BindGroup/Descriptor-Set) oder ein **Bindless-Index** sein (siehe Abschnitt 6.1) – das Material entscheidet pro Parameter, welcher Modus genutzt wird.

---

## 6.1 Bindless Textures

Ziel: Texturen nicht pro Draw über ein BindGroup/Descriptor-Set binden, sondern per Index in einer globalen Textur-Tabelle ansprechen (Vulkan `VK_EXT_descriptor_indexing`).

```cpp
class BindlessTextureHeap {
public:
    uint32_t allocate(TextureHandle texture);   // globaler Slot-Index
    void free(uint32_t index);
    void update(uint32_t index, TextureHandle newTexture);
    BindGroupHandle bindGroup() const;          // 1x pro Frame gebunden, festes Set
private:
    std::vector<TextureHandle> slots;
    std::vector<uint32_t> freeSlots;
};
```

Material-Parameter referenzieren dann nur einen `uint32_t textureIndex` (z.B. per Push-Constant/Instance-Daten an den Shader übergeben), statt ein Textur-Binding pro Draw zu wechseln. Das reduziert State-Changes drastisch und ist Voraussetzung für effizientes Batching großer Sprite-Mengen mit unterschiedlichen Texturen.

**Vulkan (primäres Backend):** Echtes, unbegrenztes dynamisches Indexing via `VK_EXT_descriptor_indexing` (`VkDescriptorBindingFlagBits::VARIABLE_DESCRIPTOR_COUNT` + `PARTIALLY_BOUND_BIT`) – funktioniert von Anfang an ohne Einschränkung.

**WebGPU-Export-Target (Web, später):** Dort ist echtes dynamisches Indexing in Binding-Arrays nicht überall verfügbar (abhängig von Dawn/Browser-Support). Für den Web-Build fällt `BindlessTextureHeap` daher auf einen **festen Textur-Array-Slot-Pool** (z.B. 256–1024 Slots) zurück – eine reine Web-Export-Einschränkung, die den Desktop-Pfad (Vulkan) nicht betrifft.

---

## 7. RenderLayer (dynamisch, editor-definierbar)

**Keine fest hartcodierten Layer.** Layer-Namen werden im Editor definiert und über eine Registry auf Bits einer 32-bit-Bitmask gemappt. Die Bitmask bleibt zur Laufzeit performant, die Namen/Zuordnung sind Projektdaten (serialisiert, im Editor editierbar).

```cpp
using RenderLayerMask = uint32_t;

class RenderLayerRegistry {
public:
    uint8_t registerLayer(const std::string& name);      // vergibt nächstes freies Bit (0-31)
    void unregisterLayer(const std::string& name);
    RenderLayerMask maskFor(const std::string& name) const;
    const std::string& nameOf(uint8_t bit) const;
    // Serialisierung: Projekt-Config <-> Registry
};
```

Layer-spezifische Post-Effekte (z.B. "alles außer Characters schwärzen") abonnieren eine `RenderLayerMask`, nicht einen Namen zur Compile-Zeit.

---

## 8. ECS-Integration

```cpp
struct MaterialComponent {
    std::shared_ptr<Material> material;
};

struct RenderLayerComponent {
    RenderLayerMask mask; // optional, manuell gesetzt
};

// Geometrie-Komponenten (siehe Abschnitt 9), z.B.:
struct SpriteComponent { TextureHandle texture; glm::vec2 size; Rect uv; };
struct SkinnedMeshComponent { MeshHandle mesh; SkeletonHandle skeleton; };
struct CircleComponent { float radius; uint32_t segments; };
struct LineStripComponent { std::vector<glm::vec2> points; float width; JoinType join; CapType cap; };
```

**Verhalten `RenderSubmissionSystem`:**
- Wird ein `MaterialComponent` im Editor zu einem Entity (z.B. Sprite) hinzugefügt, wird es **automatisch** beim Rendern mit diesem Objekt assoziiert – kein manueller Registrierungsschritt nötig.
- Ist kein `MaterialComponent` vorhanden, wird das Default-Material für den jeweiligen Geometrie-Typ verwendet.
- Ist ein `RenderLayerComponent` vorhanden, wird dessen Maske genutzt; sonst der Default-Layer der Registry.
- So lassen sich gezielt einzelne Entities einem bestimmten Layer/Material zuordnen, während der Rest automatisch läuft.
- **Zuständigkeit endet bei der Collection.** Das System baut `RenderItem`s aus Components und ruft `Renderer::submitItem(item)` auf – es kennt weder Pass-Deskriptoren noch Sortiermodi. Sortierung ist Renderer-Sache (siehe Abschnitt 10), damit auch Nicht-ECS-Submitter (Editor-Gizmos, Debug-Draws) automatisch korrekt behandelt werden.

`RenderItem` ist die gemeinsame Datenstruktur, die aus ECS-Komponenten gebaut und an den Renderer übergeben wird:

```cpp
struct RenderItem {
    RenderLayerMask layer;
    int32_t zIndex;       // relevant für 2D/UI
    float depth;          // relevant für 3D-Transparenz (View-Space-Tiefe)
    ShaderID shaderId;
    std::shared_ptr<Material> material;
    BufferHandle vertexBuffer, indexBuffer;
    uint32_t indexCount;
    glm::mat4 transform;
};
```

---

## 9. 2D-Primitive (erste Zielsetzung: komplette 2D-Szene)

Linienartige Primitive (Linie/Linestrip, Rechteck-Outline) werden als **echte Geometrie (Dreiecke)** erzeugt, nicht als GPU-native Line-Primitives – analog zu SVG-Stroke-Rendering, damit Breite, Joins und Caps korrekt funktionieren. **Der Kreis ist die Ausnahme:** er wird nicht tesselliert, sondern als **2D-SDF auf einem Quad** gezeichnet.

| Primitiv | Beschreibung |
|---|---|
| **Skinned Mesh (2D)** | Vertex-Skinning über Bone-Weights/Indices im Vertexformat |
| **Kreis** | Quad-Geometrie (2 Dreiecke) + Signed-Distance-Field im Fragment-Shader, kein Tesselations-Aufwand |
| **Linie / Linestrip** | Stroke-Tesselation zu Dreiecksgeometrie; Endcaps: `Butt`, `Round`, `Square`; Joins: `Miter`, `Round`, `Bevel` – wie HTML Canvas/SVG |
| **Rechteck** | Nur Outline (Stroke), aus Linien-Tesselator abgeleitet (4 Segmente + Eck-Joins) |
| **Quad/Sprite** | Gefüllt, mit oder ohne Textur |

**Kreis als SDF:** Die Geometrie ist ein simples Quad (dieselbe wie beim Sprite), die eigentliche Form entsteht im Fragment-Shader über die Distanzfunktion zum Kreismittelpunkt. Zwei Parameter steuern das Aussehen:

- **`thickness`** (Bereich `[0, 1]`): `1.0` = vollständig gefüllte Fläche (Disk). Werte `< 1.0` machen aus dem Kreis einen Ring – `thickness` bestimmt die Breite des Rings relativ zum Radius (also wie weit die innere Kante vom Rand entfernt ist).
- **`feather`**: Weichheit der Kante (Anti-Aliasing/weicher Verlauf statt hartem Cutoff), typischerweise als Distanz in normalisierten UV-Einheiten oder Pixeln.

```cpp
struct CircleComponent {
    float radius;
    float thickness = 1.0f; // 1.0 = geschlossene Fläche, <1.0 = Ring
    float feather = 0.01f;  // Kantenweichheit
};
```

Fragment-Shader-Prinzip (WGSL-artiges Pseudo):
```
let dist = length(localUV);              // 0 im Zentrum, 1 am Kreisrand
let innerEdge = 1.0 - thickness;         // 0.0 bei thickness=1 (voller Disk)
let outerAlpha = 1.0 - smoothstep(1.0 - feather, 1.0, dist);
let innerAlpha = select(1.0, smoothstep(innerEdge - feather, innerEdge, dist), thickness < 1.0);
let alpha = outerAlpha * innerAlpha;
```

Da der Kreis nur ein Quad + Material ist, läuft er durch denselben Submission-/Sortier-/Batching-Pfad wie jedes andere `RenderItem` – kein eigener Geometrie-Builder nötig, nur ein eigenes Kreis-Material/-Shader mit `thickness`/`feather` als Uniform-Parameter.

Ein `GeometryBuilder` erzeugt aus den High-Level-Beschreibungen (Punkte, Breite, Join/Cap-Typ) Vertex-/Index-Buffer für die linienartigen Primitive:

```cpp
class LineStrokeBuilder {
public:
    static MeshData build(std::span<const glm::vec2> points, float width,
                           JoinType join, CapType cap);
};
```

Alle Primitive laufen anschließend durch denselben Submission-/Batching-Pfad wie jedes andere `RenderItem`.

---

## 10. Sortierung & Batching – eigenständige Subsysteme, nicht Teil des RenderGraph

**Weder Sortierung noch Batching gehören zum `RenderGraph`.** Der Graph plant nur Passes/Ressourcen (Abschnitt 3). Sortierung und Batching sind zwei separate, kleine Subsysteme, die ein Pass in seiner `execute()`-Implementierung nutzt – austauschbar, unabhängig testbar, ohne dass der Graph selbst sie kennen muss.

```cpp
enum class SortMode {
    None,            // keine Sortierung nötig
    PipelineGrouped, // nur nach Shader/Material gruppiert (z.B. 3D-Opaque)
    FrontToBack,     // für Early-Z-Optimierung (3D-Opaque, optional)
    BackToFront      // für korrektes Blending (2D, UI, 3D-Transparent)
};

struct RenderQueueDescriptor {
    RenderLayerMask acceptedLayers;
    SortMode sortMode;
};
```

**1. `RenderQueueSystem`** – sitzt zwischen Item-Submission und Graph-Ausführung, nicht im Graph selbst:

```cpp
class RenderQueueSystem {
public:
    // Läuft einmal pro Frame, bevor RenderGraph.compile() aufgerufen wird.
    // Liefert pro Pass eine bereits gefilterte + sortierte Item-Liste.
    std::unordered_map<PassID, std::vector<RenderItem>>
    build(std::span<const RenderItem> itemPool,
          std::span<const RenderQueueDescriptor> passDescriptors);
};

namespace RenderSort {
    void sort(std::vector<RenderItem>& items, SortMode mode);
}
```

**2. `BatchBuilder`** – wird von einem Pass in `execute()` aufgerufen, um aus einer bereits sortierten Item-Liste instanced Draw-Batches zu bauen:

```cpp
struct DrawBatch {
    ShaderID shaderId;
    std::shared_ptr<Material> material;
    BufferHandle vertexBuffer, indexBuffer;
    uint32_t indexCount;
    std::vector<glm::mat4> instanceTransforms; // oder Instance-Buffer-Handle
};

namespace BatchBuilder {
    std::vector<DrawBatch> build(std::span<const RenderItem> sortedItems);
}
```

**Gesamtablauf für einen Pass** (z.B. `Pass2D::execute`):

```cpp
void Pass2D::execute(RenderContext& ctx, CommandList& cmd) {
    std::span<const RenderItem> items = ctx.queueSystem.itemsFor(passId());
    // items sind bereits gefiltert + sortiert (RenderQueueSystem lief vor RenderGraph.compile())
    std::vector<DrawBatch> batches = BatchBuilder::build(items);
    for (auto& batch : batches) {
        cmd.bindPipeline(resolvePipeline(batch));
        cmd.bindVertexBuffer(0, batch.vertexBuffer);
        cmd.drawIndexed(batch.indexCount, uint32_t(batch.instanceTransforms.size()), 0);
    }
}
```

Sort-Key innerhalb einer `BackToFront`/`PipelineGrouped`-Sortierung bleibt ein zusammengesetzter Key (Layer → Shader → Material → Z-Index/Tiefe):

```cpp
uint64_t sortKey = (uint64_t(layer) << 48) | (uint64_t(shaderId) << 32)
                 | (uint64_t(materialId) << 16) | zIndexBits;
```

**Kurz zusammengefasst, wer was macht:**

| Schritt | Komponente |
|---|---|
| Components → `RenderItem` bauen | ECS (`RenderSubmissionSystem`) |
| Nach Layer filtern + sortieren | `RenderQueueSystem` (eigenständig, vor Graph-Compile) |
| Pass-Scheduling, Barriers, Aliasing | `RenderGraph` (kennt keine RenderItems) |
| Batchen (instanced Draws) | `BatchBuilder`, aufgerufen aus `Pass::execute()` |
| CommandList/GPU-Submit | Backend (`IRHIBackend`) |

---

## 11. Views, Kameras & Render-to-Texture

**Eine `View` ist Kamera + Ziel-Textur + sichtbare Layer.** Der Renderer kennt beliebig viele registrierte Views und führt den `RenderGraph` **pro View einmal komplett aus** – z.B. eine Game-View auf den Swapchain und gleichzeitig eine Editor-View auf eine Offscreen-Textur, die der Editor dann z.B. in ein UI-Fenster zeichnet.

```cpp
struct Viewport { uint32_t x, y, width, height; };

enum class RenderTargetKind { Swapchain, Texture };

struct RenderTarget {
    RenderTargetKind kind;
    TextureHandle texture; // nur gültig wenn kind == Texture
};

struct View {
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;   // orthographic (2D) oder perspective (3D)
    RenderLayerMask visibleLayers;
    RenderTarget target;
    Viewport viewport;
    int priority = 0;             // Ausführungsreihenfolge unter mehreren Views
};

using ViewID = uint32_t;

class Renderer {
public:
    ViewID registerView(const View& view);
    void updateView(ViewID, const View&);
    void removeView(ViewID);
    void renderFrame(); // iteriert alle Views nach priority, führt Graph pro View aus
};
```

**Frame-Ablauf pro View:**

```
für jede View (sortiert nach priority):
    RenderContext.currentView = view
    RenderQueueSystem.build(itemPool, passDescriptors, view)
        → filtert zusätzlich zu descriptor.acceptedLayers auch nach view.visibleLayers
        → optional: Frustum-Culling gegen view.viewMatrix/projectionMatrix
    RenderGraph.compile()/execute() für diese View
        → transiente Resourcen (z.B. "SceneColor") sind PRO VIEW neu angelegt,
          nicht global geteilt – außer ein Pass registriert sie explizit
          als frame-shared (z.B. eine einmal berechnete Shadow Map für alle Views)
        → letzter Pass schreibt in view.target (Swapchain oder Texture)
```

Die `acceptedLayers` eines Passes (Abschnitt 10) und die `visibleLayers` einer View werden kombiniert (UND-verknüpft) – ein Pass sieht also nur Items, die sowohl für ihn als auch für die aktuelle View bestimmt sind.

**Beispiel – Editor-Kamera neben Game-Kamera:**

```cpp
View gameView {
    .viewMatrix = camera.view(), .projectionMatrix = camera.projection(),
    .visibleLayers = gameplayLayers,
    .target = { RenderTargetKind::Swapchain, {} },
    .priority = 0
};

View editorView {
    .viewMatrix = editorCam.view(), .projectionMatrix = editorCam.projection(),
    .visibleLayers = gameplayLayers | gizmoLayer,   // sieht zusätzlich Editor-Gizmos
    .target = { RenderTargetKind::Texture, editorViewportTexture },
    .priority = 1
};

renderer.registerView(gameView);
renderer.registerView(editorView);
```

Beide Views laufen durch denselben `RenderGraph`/dieselben Passes – kein separater "Editor-Renderer" nötig. Render-to-Texture ist damit generell verfügbar: Minimap-Kamera, Portrait-Render für ein Inventar-UI, Editor-Viewport – alles einfach eine `View` mit `target.kind = Texture` statt `Swapchain`.

**FXAA als Beispiel-Pass (funktioniert identisch für 2D und 3D):**

```cpp
class FXAAPass : public RenderPass {
public:
    void setup(RenderGraphBuilder& builder) override {
        m_input = builder.read("SceneColor");        // von Pass2D UND/ODER Pass3D geschrieben
        m_output = builder.write(currentViewTarget());
    }
    void execute(RenderContext& ctx, CommandList& cmd) override {
        cmd.bindPipeline(m_fxaaPipeline);
        cmd.bindGroup(0, resolveBindGroup(m_input));
        cmd.draw(3, 1, 0, 0); // Fullscreen-Triangle
    }
};
```

Da sowohl `Pass2D` als auch `Pass3DOpaque`/`Pass3DTransparent` in dieselbe `"SceneColor"`-Resource schreiben können (der Graph unterstützt mehrere Writer mit Dependency-Ordering, Abschnitt 3), braucht FXAA keinen Sonderfall für 2D vs. 3D – es ist einfach ein weiterer Pass, der diese Resource liest und in `view.target` schreibt.

---

## 12. Post-Effekte pro Layer

Ein `PostEffectPass` bekommt eine `RenderLayerMask targetLayers`. Items, deren Layer in der Maske eines isolierenden Effekts liegen, werden in ein separates Zwischentarget gerendert, das der Pass verarbeitet, bevor final compositiert wird (z.B. "alles schwarz außer Characters-Layer").

---

## 13. Roadmap

**Phase 1 – RHI-Fundament (Vulkan)**
1. `IRHIBackend`-Interface definieren (zunächst nur Buffers, Textures – Pipelines/BindGroups folgen in Phase 3, sobald `VertexLayout`/`ShaderDesc` existieren)
2. Vulkan-Implementierung: Instance/Device/Queue-Setup, `CommandList` → `VkCommandBuffer` (zunächst nur Copy-Commands, `bindPipeline`/`draw`/etc. folgen in Phase 3)
3. Buffer-/Texture-Upload-Pfad (explizite Staging-Buffer + Copy-Commands, da Vulkan – anders als Dawns `Queue::WriteBuffer` – kein automatisches Staging übernimmt)

**Phase 2 – RenderGraph-Grundgerüst**
4. `RenderPass`-Interface + `RenderGraph` (addPass, compile, execute)
5. Virtuelle Resource-Handles + einfaches Transient-Aliasing
6. Ein minimaler Test-Pass (Clear-Screen) end-to-end durchs Backend

**Phase 3 – Material, Pipelines & Bindless**
7. `VertexLayout` + `ShaderDesc` (Abschnitt 5/6) definieren – Shader werden offline zu SPIR-V kompiliert (`glslc` aus dem Vulkan SDK), keine Runtime-Kompilierung; `ShaderDesc` referenziert kompilierte `.spv`-Dateien
8. `IRHIBackend::createPipeline(const PipelineDesc&)` + `PipelineDesc` (baut auf `VertexLayout`/`ShaderDesc` auf)
9. `IRHIBackend`: BindGroup-Erzeugung (`createBindGroup`) ergänzen
10. `CommandList` vervollständigen: `bindPipeline`, `bindVertexBuffer`, `bindIndexBuffer`, `bindGroup`, `draw`, `drawIndexed`, `dispatch`
11. `Material`-Klasse + `ShaderID`-Verwaltung
12. `BindlessTextureHeap` (Vulkan Descriptor Indexing als Primärpfad, siehe Abschnitt 6.1)
13. `ResourceBinding`: direktes Binding vs. Bindless-Index

**Phase 4 – RenderLayer & ECS-Anbindung**
14. `RenderLayerRegistry` (Bit-Vergabe, Serialisierung; Editor-UI folgt separat)
15. `MaterialComponent` / `RenderLayerComponent` + `RenderSubmissionSystem` (Auto-Association-Logik)

**Phase 5 – 2D-Geometrie**
16. `GeometryBuilder`: Quad (mit/ohne Textur) zuerst, da einfachster Fall
17. Kreis-SDF-Shader/-Material (`thickness`/`feather` als Uniform-Parameter, nutzt dieselbe Quad-Geometrie, kein eigener Builder nötig)
18. `LineStrokeBuilder`: erst Linien ohne Joins/Caps, dann Miter/Round/Bevel-Joins, dann Butt/Round/Square-Caps
19. Rechteck-Outline (Wiederverwendung von `LineStrokeBuilder`, 4 Segmente + Eckbehandlung)
20. 2D-Skinned-Mesh (Bone-Weights im Vertexformat, CPU- oder Shader-Skinning entscheiden)

**Phase 6 – Queue-System, Batching, konkrete Passes**
21. `RenderQueueDescriptor` pro Pass + `RenderSort`-Utility (Sortier-Algorithmen als eigenständiger, testbarer Baustein)
22. `RenderQueueSystem`: eigenständiges Subsystem, läuft vor `RenderGraph.compile()`, filtert + sortiert Item-Pool pro Pass – kein Teil des Graphs
23. `RenderSubmissionSystem` (ECS): nur Collection – Components lesen, `RenderItem`s bauen, roh via `Renderer::submitItem(...)` abliefern
24. `BatchBuilder`: eigenständige Utility, baut instanced Draw-Batches aus sortierten Item-Listen, aufgerufen aus `Pass::execute()`
25. `Pass2D` (SortMode `BackToFront`) nutzt `BatchBuilder` intern
26. `PassUI` als separater Pass mit eigenem Deskriptor

**Phase 7 – Views, Render-to-Texture & FXAA**
27. `View`/`RenderTarget`-Struktur + `Renderer::registerView/updateView/removeView`
28. `Renderer::renderFrame()`: iteriert Views nach `priority`, führt Graph pro View aus
29. `RenderQueueSystem` um View-Filterung erweitern (`acceptedLayers` UND `visibleLayers`)
30. Render-to-Texture end-to-end testen: eine View auf Swapchain, eine zweite auf Offscreen-Textur (z.B. simulierter Editor-Viewport)
31. `FXAAPass`: liest `SceneColor`, schreibt in `view.target` – Test mit Pass2D-Output

**Phase 8 – Post-FX (Layer-Masking)**
32. `PostEffectPass`-Infrastruktur mit `RenderLayerMask`-Filter
33. Beispiel-Effekt: Layer-Isolation (z.B. alles außer Characters schwärzen)

**Phase 9 – Erste komplette 2D-Szene**
34. End-to-End-Test: mehrere Layer, gemischte Materialien, Post-FX, FXAA, Batching, Editor-View + Game-View gleichzeitig – alles über Vulkan

**Phase 10 – Danach**
35. 3D-Passes (Opaque/Transparent) inkl. Front-to-Back-Sortierung für Early-Z
36. Compute-Shader-getriebene Effekte (Fluid, Deformation – Anknüpfung an bestehende Engine-Systeme)
37. **WebGPU-Backend (Dawn) für Web-Export-Target**: hinter demselben `IRHIBackend`-Interface, ausschließlich für Browser/WASM-Builds – inkl. Bindless-Fallback auf festen Slot-Pool (Abschnitt 6.1)

---

*Letzte Aktualisierung: siehe Commit-Historie / Konversationsverlauf mit Claude.*