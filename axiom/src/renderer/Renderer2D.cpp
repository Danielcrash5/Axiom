#include "axiom/renderer/Renderer2D.h"
#include "axiom/renderer/Renderer.h"
#include "axiom/renderer/VertexArray.h"
#include "axiom/renderer/VertexBuffer.h"
#include "axiom/renderer/IndexBuffer.h"
#include "axiom/renderer/Shader.h"
#include "axiom/renderer/Texture.h"
#include "axiom/renderer/VertexBufferLayout.h"
#include <algorithm>

namespace axiom {

    std::vector<Renderer2D::RenderItem> Renderer2D::s_Items;
    glm::mat4 Renderer2D::s_ViewProjection;

    // Definitions for static members declared in the header
    Renderer2D::QuadVertex* Renderer2D::s_VertexBufferBase = nullptr;
    Renderer2D::QuadVertex* Renderer2D::s_VertexBufferPtr = nullptr;

    uint32_t Renderer2D::s_IndexCount = 0;

    std::shared_ptr<VertexArray> Renderer2D::s_QuadVAO = nullptr;
    std::shared_ptr<VertexBuffer> Renderer2D::s_VertexBuffer = nullptr;
    std::shared_ptr<IndexBuffer> Renderer2D::s_IndexBuffer = nullptr;

    std::shared_ptr<Material> Renderer2D::s_DefaultQuadMaterial = nullptr;
    std::shared_ptr<Material> Renderer2D::s_DefaultCircleMaterial = nullptr;
    std::shared_ptr<Material> Renderer2D::s_DefaultLineMaterial = nullptr;
    std::shared_ptr<Texture> Renderer2D::s_TextureSlots[Renderer2D::MaxTextureSlots] = { nullptr };

    int Renderer2D::bindTextureToSlot(std::shared_ptr<Texture> texture) {
        if (!texture) return -1;

        // check if already bound
        for (uint32_t i = 0; i < MaxTextureSlots; i++) {
            if (s_TextureSlots[i] && s_TextureSlots[i]->getRendererID() == texture->getRendererID())
                return (int)i;
        }

        // find free slot
        for (uint32_t i = 0; i < MaxTextureSlots; i++) {
            if (!s_TextureSlots[i]) {
                s_TextureSlots[i] = texture;
                return (int)i;
            }
        }

        // no slot available
        return -1;
    }

    void Renderer2D::Init() {
        s_VertexBufferBase = new QuadVertex[MaxVertices];

        s_QuadVAO = std::make_shared<VertexArray>();

        s_VertexBuffer = std::make_shared<VertexBuffer>(MaxVertices * sizeof(QuadVertex));

        VertexBufferLayout layout;
        layout.push<float>(3); // position
        layout.push<float>(2); // uv
        layout.push<float>(4); // color
        layout.push<float>(1); // tex index

        s_QuadVAO->setVertexBuffer(*s_VertexBuffer, layout);

        // Index Buffer
        uint32_t* indices = new uint32_t[MaxIndices];

        uint32_t offset = 0;
        for (uint32_t i = 0; i < MaxIndices; i += 6) {
            indices[i + 0] = offset + 0;
            indices[i + 1] = offset + 1;
            indices[i + 2] = offset + 2;

            indices[i + 3] = offset + 2;
            indices[i + 4] = offset + 3;
            indices[i + 5] = offset + 0;

            offset += 4;
        }

        s_IndexBuffer = std::make_shared<IndexBuffer>(indices, MaxIndices);
        s_QuadVAO->setIndexBuffer(*s_IndexBuffer);

        delete[] indices;

        // Default Materials erstellen
        // Load shaders from assets/shaders/Renderer2D
        auto quadShader = std::make_shared<Shader>();
        quadShader->compile({ { ShaderStage::Vertex, "assets/shaders/Renderer2D/Quad.vert.glsl" },
                             { ShaderStage::Fragment, "assets/shaders/Renderer2D/Quad.frag.glsl" } });

        auto circleShader = std::make_shared<Shader>();
        circleShader->compile({ { ShaderStage::Vertex, "assets/shaders/Renderer2D/Circle.vert.glsl" },
                               { ShaderStage::Fragment, "assets/shaders/Renderer2D/Circle.frag.glsl" } });

        s_DefaultQuadMaterial = std::make_shared<Material>(quadShader);
        s_DefaultQuadMaterial->setVec4("u_Color", glm::vec4(1.0f));
        s_DefaultQuadMaterial->setInt("u_UseTexture", 0);

        s_DefaultCircleMaterial = std::make_shared<Material>(circleShader);
        s_DefaultCircleMaterial->setVec4("u_Color", glm::vec4(1.0f));
        s_DefaultCircleMaterial->setFloat("u_Thickness", 0.0f);
        s_DefaultCircleMaterial->setFloat("u_Feather", 0.005f);
    }

    void Renderer2D::Shutdown() {
    }

    void Renderer2D::Begin(const Camera& camera) {
        s_ViewProjection = camera.getViewProjectionMatrix();
        s_Items.clear();

        // Reset batch write pointer and index count
        s_VertexBufferPtr = s_VertexBufferBase;
        s_IndexCount = 0;

        // Render State
        RenderState state;
        state.DepthTest = true;
        state.DepthWrite = true;
        state.Blend = true;
        state.CullFace = false;
        state.BlendSrc = GL_SRC_ALPHA;
        state.BlendDst = GL_ONE_MINUS_SRC_ALPHA;

        Renderer::SetRenderState(state);

        // clear texture slots
        for (uint32_t i = 0; i < MaxTextureSlots; i++)
            s_TextureSlots[i] = nullptr;
    }

    void Renderer2D::End() {
        // Sort by material pointer
        std::sort(s_Items.begin(), s_Items.end(),
                  [](const RenderItem& a, const RenderItem& b) {
                      return a.Material.get() < b.Material.get();
                  });

        // Simple batch: for each item, write vertices into the buffer and draw
        for (auto& item : s_Items) {
            // choose default material by type when none provided
            std::shared_ptr<Material> mat = item.Material;
            if (!mat) {
                if (item.Type == 0) mat = s_DefaultQuadMaterial;
                else if (item.Type == 1) mat = s_DefaultCircleMaterial;
                else mat = s_DefaultLineMaterial ? s_DefaultLineMaterial : s_DefaultQuadMaterial;
            }
            if (!mat) continue;

            // bind textures used by material and ensure they are in texture slots
            int primaryTexSlot = -1;
            for (auto& tb : mat->getTextureBindings()) {
                const std::string& name = std::get<0>(tb);
                std::shared_ptr<Texture> tex = std::get<1>(tb);
                int slot = bindTextureToSlot(tex);
                if (slot >= 0) {
                    mat->setTextureSlot(name, slot);
                    mat->setInt(name, slot);
                    if (primaryTexSlot == -1) primaryTexSlot = slot;
                }
            }

            // prepare 4 vertices for a quad centered at origin with size 1
            // positions are in local space; shader will transform
            float texIdx = (primaryTexSlot >= 0) ? (float)primaryTexSlot : -1.0f;
            QuadVertex v0{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f }, glm::vec4(1.0f), texIdx };
            QuadVertex v1{ {  0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f }, glm::vec4(1.0f), texIdx };
            QuadVertex v2{ {  0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f }, glm::vec4(1.0f), texIdx };
            QuadVertex v3{ { -0.5f,  0.5f, 0.0f }, { 0.0f, 1.0f }, glm::vec4(1.0f), texIdx };

            // write vertices
            *s_VertexBufferPtr++ = v0;
            *s_VertexBufferPtr++ = v1;
            *s_VertexBufferPtr++ = v2;
            *s_VertexBufferPtr++ = v3;

            // update GPU buffer with all written data so far
            uint32_t usedVerts = (uint32_t)(s_VertexBufferPtr - s_VertexBufferBase);
            s_VertexBuffer->setData(s_VertexBufferBase, usedVerts * sizeof(QuadVertex));

            // bind material and set common uniforms
            mat->bind();
            mat->setMat4("u_ViewProjection", s_ViewProjection);
            mat->setMat4("u_Transform", item.Transform);

            // draw the quad (6 indices)
            Renderer::DrawIndexed(*s_QuadVAO, 6);
        }
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, std::shared_ptr<Material> material) {
        s_Items.push_back({ transform, material, 0 });
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, float rotation, std::shared_ptr<Material> material) {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0,0,1))
            * glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));

        if (!material) material = s_DefaultQuadMaterial;
        s_Items.push_back({ transform, material, 0 });
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, std::shared_ptr<Texture> texture) {
        std::shared_ptr<Shader> shader = nullptr;
        if (s_DefaultQuadMaterial) shader = s_DefaultQuadMaterial->getShader();
        auto mat = std::make_shared<Material>(shader);
        if (mat && texture) mat->setTexture("u_Texture", texture, 0);
        DrawQuad(position, size, 0.0f, mat);
    }

    void Renderer2D::DrawQuadOutline(const glm::vec3& position, const glm::vec2& size, float rotation, std::shared_ptr<Material> material, float thickness) {
        if (!material) material = s_DefaultLineMaterial ? s_DefaultLineMaterial : s_DefaultQuadMaterial;

        // draw four lines around the quad
        glm::vec3 half = glm::vec3(size.x * 0.5f, size.y * 0.5f, 0.0f);
        glm::mat4 rot = glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0,0,1));

        glm::vec3 corners[4] = {
            position + glm::vec3(rot * glm::vec4(-half,1.0f)),
            position + glm::vec3(rot * glm::vec4( half.x,-half.y,0.0f,1.0f)),
            position + glm::vec3(rot * glm::vec4( half,1.0f)),
            position + glm::vec3(rot * glm::vec4(-half.x, half.y,0.0f,1.0f))
        };

        for (int i = 0; i < 4; i++) {
            DrawLine(corners[i], corners[(i+1)%4], material);
        }
    }

    void Renderer2D::DrawCircle(const glm::mat4& transform, std::shared_ptr<Material> material) {
        s_Items.push_back({ transform, material, 1 });
    }

    void Renderer2D::DrawCircle(const glm::vec3& position, float radius, std::shared_ptr<Material> material) {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), glm::vec3(radius * 2.0f, radius * 2.0f, 1.0f));

        if (!material) material = s_DefaultCircleMaterial;
        s_Items.push_back({ transform, material, 1 });
    }

    void Renderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, std::shared_ptr<Material> material, float thickness, Renderer2D::LineCap cap) {
        glm::vec3 dir = p1 - p0;
        float length = glm::length(dir);
        if (!material) material = s_DefaultLineMaterial ? s_DefaultLineMaterial : s_DefaultQuadMaterial;

        glm::vec3 mid = p0 + dir * 0.5f;
        float angle = atan2(dir.y, dir.x);

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), mid)
            * glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0,0,1))
            * glm::scale(glm::mat4(1.0f), { length, thickness, 1.0f });

        RenderItem it;
        it.Transform = transform;
        it.Material = material;
        it.Type = 2;
        it.Thickness = thickness;
        it.Points = { p0, p1 };
        // encode cap in color.w (cheap hack) if needed later
        it.Color.w = (cap == LineCap::Round) ? 1.0f : 0.0f;
        s_Items.push_back(std::move(it));
    }

    void Renderer2D::DrawLineStrip(const std::vector<glm::vec3>& points, std::shared_ptr<Material> material, float thickness, Renderer2D::LineJoin join, Renderer2D::LineCap cap) {
        if (points.size() < 2) return;
        if (!material) material = s_DefaultLineMaterial ? s_DefaultLineMaterial : s_DefaultQuadMaterial;

        // Immediate CPU-generated geometry for the entire strip using miter/bevel joins
        const float half = thickness * 0.5f;
        const float miterLimit = 4.0f; // max miter length in multiples of half

        std::vector<QuadVertex> verts;
        std::vector<std::shared_ptr<Material>> materials; // parallel per segment

        size_t n = points.size();
        for (size_t i = 0; i < n - 1; ++i) {
            glm::vec3 p0 = points[i];
            glm::vec3 p1 = points[i + 1];

            glm::vec2 dir = glm::normalize(glm::vec2(p1 - p0));
            glm::vec2 n_curr = glm::vec2(-dir.y, dir.x);

            // compute start offset (at p0)
            glm::vec2 startOffset2D;
            if (i == 0) {
                startOffset2D = n_curr * half;
            } else {
                glm::vec3 pprev = points[i - 1];
                glm::vec2 prevDir = glm::normalize(glm::vec2(p0 - pprev));
                glm::vec2 n_prev = glm::vec2(-prevDir.y, prevDir.x);
                glm::vec2 sum = glm::normalize(n_prev + n_curr);
                float denom = glm::dot(sum, n_curr);
                if (glm::length(n_prev + n_curr) < 1e-6f || fabs(denom) < 1e-6f) {
                    startOffset2D = n_curr * half;
                } else {
                    float miterLen = half / denom;
                    if (join == LineJoin::Miter && fabs(miterLen) <= half * miterLimit) {
                        startOffset2D = sum * miterLen;
                    } else if (join == LineJoin::Bevel || fabs(miterLen) > half * miterLimit) {
                        startOffset2D = n_curr * half; // bevel
                    } else {
                        // round fallback: approximate with bevel for now
                        startOffset2D = n_curr * half;
                    }
                }
            }

            // compute end offset (at p1)
            glm::vec2 endOffset2D;
            if (i + 1 == n - 1) {
                endOffset2D = n_curr * half;
            } else {
                glm::vec3 pnext = points[i + 2];
                glm::vec2 nextDir = glm::normalize(glm::vec2(pnext - p1));
                glm::vec2 n_next = glm::vec2(-nextDir.y, nextDir.x);
                glm::vec2 sum = glm::normalize(n_curr + n_next);
                float denom = glm::dot(sum, n_next);
                if (glm::length(n_curr + n_next) < 1e-6f || fabs(denom) < 1e-6f) {
                    endOffset2D = n_curr * half;
                } else {
                    float miterLen = half / denom;
                    if (join == LineJoin::Miter && fabs(miterLen) <= half * miterLimit) {
                        endOffset2D = sum * miterLen;
                    } else if (join == LineJoin::Bevel || fabs(miterLen) > half * miterLimit) {
                        endOffset2D = n_curr * half;
                    } else {
                        endOffset2D = n_curr * half;
                    }
                }
            }

            // build quad vertices for this segment
            glm::vec3 sOff3 = glm::vec3(startOffset2D, 0.0f);
            glm::vec3 eOff3 = glm::vec3(endOffset2D, 0.0f);

            QuadVertex v0{ p0 - sOff3, {0.0f, 0.0f}, glm::vec4(1.0f), -1.0f };
            QuadVertex v1{ p1 - eOff3, {1.0f, 0.0f}, glm::vec4(1.0f), -1.0f };
            QuadVertex v2{ p1 + eOff3, {1.0f, 1.0f}, glm::vec4(1.0f), -1.0f };
            QuadVertex v3{ p0 + sOff3, {0.0f, 1.0f}, glm::vec4(1.0f), -1.0f };

            verts.push_back(v0);
            verts.push_back(v1);
            verts.push_back(v2);
            verts.push_back(v3);
            // store material per segment
            materials.push_back(material);
        }

        if (verts.empty()) return;

        // upload vertices
        s_VertexBuffer->setData(verts.data(), (uint32_t)verts.size() * sizeof(QuadVertex));

        // Batch draw grouped by material: we created 'materials' parallel to segments
        size_t segmentCount = materials.size();
        if (segmentCount == 0) return;

        // upload all verts once
        s_VertexBuffer->setData(verts.data(), (uint32_t)verts.size() * sizeof(QuadVertex));

        // group segments by material pointer to minimize binds/draws
        size_t segIndex = 0;
        while (segIndex < segmentCount) {
            auto currentMat = materials[segIndex];
            size_t startSeg = segIndex;
            while (segIndex < segmentCount && materials[segIndex] == currentMat) segIndex++;
            size_t countSegs = segIndex - startSeg;

            // bind textures for this material
            if (currentMat) {
                for (auto& tb : currentMat->getTextureBindings()) {
                    const std::string& name = std::get<0>(tb);
                    std::shared_ptr<Texture> tex = std::get<1>(tb);
                    int slot = bindTextureToSlot(tex);
                    if (slot >= 0)
                        currentMat->setInt(name, slot);
                }
                currentMat->bind();
                currentMat->setMat4("u_ViewProjection", s_ViewProjection);
            }

            // draw segments range: each segment has 6 indices
            uint32_t drawCount = (uint32_t)countSegs * 6;
            // compute index offset into index buffer (startSeg * 6)
            // Renderer::DrawIndexed uses bound VAO and global index buffer; we rely on it to draw from start.
            Renderer::DrawIndexed(*s_QuadVAO, drawCount);
        }
    }

}