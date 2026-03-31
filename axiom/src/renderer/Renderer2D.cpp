#include "axiom/renderer/Renderer2D.h"
#include "axiom/renderer/VertexArray.h"
#include "axiom/renderer/VertexBuffer.h"
#include "axiom/renderer/VertexBufferLayout.h"
#include "axiom/renderer/IndexBuffer.h"
#include "axiom/renderer/Shader.h"
#include "axiom/renderer/Texture.h"
#include "axiom/renderer/Material.h"
#include "axiom/renderer/Camera.h"
#include "axiom/renderer/Renderer.h"
#include "axiom/core/Logger.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

namespace axiom {

// Static member initialization
Renderer2D::Vertex* Renderer2D::s_VertexBuffer = nullptr;
Renderer2D::Vertex* Renderer2D::s_VertexPtr = nullptr;
uint32_t Renderer2D::s_VertexCount = 0;

std::shared_ptr<VertexArray> Renderer2D::s_VAO = nullptr;
std::shared_ptr<axiom::VertexBuffer> Renderer2D::s_VBO = nullptr;
std::shared_ptr<IndexBuffer> Renderer2D::s_IBO = nullptr;

std::shared_ptr<Shader> Renderer2D::s_QuadShader = nullptr;
std::shared_ptr<Material> Renderer2D::s_DefaultMaterial = nullptr;

std::vector<Renderer2D::RenderCommand> Renderer2D::s_Commands;
glm::mat4 Renderer2D::s_ViewProjection;

std::shared_ptr<Texture> Renderer2D::s_TextureSlots[Renderer2D::MAX_TEXTURE_SLOTS] = {};
uint32_t Renderer2D::s_TextureSlotIndex = 0;

void Renderer2D::Init() {
    AXIOM_INFO("Initializing Renderer2D");

    // Allocate vertex buffer
    s_VertexBuffer = new Vertex[MAX_VERTICES];
    s_VertexPtr = s_VertexBuffer;

    // Create VAO and VBO
    s_VAO = std::make_shared<VertexArray>();

    s_VBO = std::make_shared<axiom::VertexBuffer>(MAX_VERTICES * sizeof(Vertex));

    // Set up vertex layout
    VertexBufferLayout layout;
    layout.push<float>(3); // position
    layout.push<float>(2); // uv
    layout.push<float>(4); // color
    layout.push<float>(1); // texture index

    s_VAO->setVertexBuffer(*s_VBO, layout);

    // Create index buffer
    uint32_t* indices = new uint32_t[MAX_INDICES];
    uint32_t idx = 0;
    uint32_t vertexOffset = 0;

    // Generate indices for quads
    for (uint32_t i = 0; i < MAX_INDICES; i += 6) {
        indices[i + 0] = vertexOffset + 0;
        indices[i + 1] = vertexOffset + 1;
        indices[i + 2] = vertexOffset + 2;
        indices[i + 3] = vertexOffset + 2;
        indices[i + 4] = vertexOffset + 3;
        indices[i + 5] = vertexOffset + 0;
        vertexOffset += 4;
    }

    s_IBO = std::make_shared<IndexBuffer>(indices, MAX_INDICES);
    s_VAO->setIndexBuffer(*s_IBO);
    delete[] indices;

    // Load shader
    s_QuadShader = std::make_shared<Shader>();
    s_QuadShader->compile({
        {ShaderStage::Vertex, "assets/shaders/Renderer2D/Quad.vert.glsl"},
        {ShaderStage::Fragment, "assets/shaders/Renderer2D/Quad_Simple.frag.glsl"}
    });

    s_DefaultMaterial = std::make_shared<Material>(s_QuadShader);

    // Set up render state for 2D
    RenderState state;
    state.DepthTest = true;
    state.DepthWrite = true;
    state.Blend = true;
    state.CullFace = false;
    state.BlendSrc = GL_SRC_ALPHA;
    state.BlendDst = GL_ONE_MINUS_SRC_ALPHA;
    Renderer::SetRenderState(state);

    AXIOM_INFO("Renderer2D initialized successfully");
}

void Renderer2D::Shutdown() {
    delete[] s_VertexBuffer;
}

void Renderer2D::Begin(const Camera& camera) {
    s_ViewProjection = camera.getViewProjectionMatrix();
    s_VertexPtr = s_VertexBuffer;
    s_VertexCount = 0;
    s_Commands.clear();
    s_TextureSlotIndex = 0;

    // Clear texture slots
    for (uint32_t i = 0; i < MAX_TEXTURE_SLOTS; i++) {
        s_TextureSlots[i] = nullptr;
    }
}

void Renderer2D::End() {
    if (s_VertexCount == 0) return;

    // Upload all vertices to GPU
    s_VBO->bind();
    s_VBO->setData(s_VertexBuffer, s_VertexCount * sizeof(Vertex));

    // Execute render commands
    ExecuteCommands();
}

int Renderer2D::GetTextureSlot(std::shared_ptr<Texture> texture) {
    if (!texture) return -1;

    // Check if texture is already in a slot
    for (uint32_t i = 0; i < s_TextureSlotIndex; i++) {
        if (s_TextureSlots[i] == texture) {
            return i;
        }
    }

    // Add texture to new slot
    if (s_TextureSlotIndex >= MAX_TEXTURE_SLOTS) {
        AXIOM_WARN("Texture slot limit reached!");
        return -1;
    }

    s_TextureSlots[s_TextureSlotIndex] = texture;
    return s_TextureSlotIndex++;
}

void Renderer2D::ExecuteCommands() {
    s_DefaultMaterial->bind();
    s_DefaultMaterial->getShader()->setMat4("u_ViewProjection", s_ViewProjection);

    // Bind all textures
    for (uint32_t i = 0; i < s_TextureSlotIndex; i++) {
        if (s_TextureSlots[i]) {
            s_TextureSlots[i]->bind(i);
        }
    }

    // Execute each command
    for (auto& cmd : s_Commands) {
        s_DefaultMaterial->getShader()->setMat4("u_Transform", cmd.transform);

        if (cmd.texture) {
            int slot = GetTextureSlot(cmd.texture);
            s_DefaultMaterial->getShader()->setInt("u_TextureSlot", slot);
            s_DefaultMaterial->getShader()->setInt("u_UseTexture", 1);
        } else {
            s_DefaultMaterial->getShader()->setInt("u_UseTexture", 0);
        }

        Renderer::DrawIndexed(*s_VAO, cmd.vertexCount, cmd.vertexOffset);
    }
}

void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, int32_t depth) {
    if (s_VertexCount + 4 > MAX_VERTICES) {
        AXIOM_WARN("Vertex buffer full!");
        return;
    }

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
    transform = glm::scale(transform, glm::vec3(size.x, size.y, 1.0f));

    // Apply depth offset
    transform[3][2] = depth * 0.001f;

    // Create quad vertices
    Vertex v0 = {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}, color, -1.0f};
    Vertex v1 = {{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}, color, -1.0f};
    Vertex v2 = {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f}, color, -1.0f};
    Vertex v3 = {{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f}, color, -1.0f};

    *s_VertexPtr++ = v0;
    *s_VertexPtr++ = v1;
    *s_VertexPtr++ = v2;
    *s_VertexPtr++ = v3;

    RenderCommand cmd;
    cmd.material = s_DefaultMaterial;
    cmd.transform = transform;
    cmd.vertexCount = 6;
    cmd.vertexOffset = s_VertexCount;
    cmd.texture = nullptr;

    s_Commands.push_back(cmd);
    s_VertexCount += 4;
}

void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, std::shared_ptr<Texture> texture, int32_t depth) {
    if (s_VertexCount + 4 > MAX_VERTICES) {
        AXIOM_WARN("Vertex buffer full!");
        return;
    }

    int textureSlot = GetTextureSlot(texture);
    if (textureSlot < 0) {
        AXIOM_WARN("Could not get texture slot!");
        return;
    }

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
    transform = glm::scale(transform, glm::vec3(size.x, size.y, 1.0f));
    transform[3][2] = depth * 0.001f;

    // Create quad vertices
    Vertex v0 = {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, (float)textureSlot};
    Vertex v1 = {{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, (float)textureSlot};
    Vertex v2 = {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, (float)textureSlot};
    Vertex v3 = {{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, (float)textureSlot};

    *s_VertexPtr++ = v0;
    *s_VertexPtr++ = v1;
    *s_VertexPtr++ = v2;
    *s_VertexPtr++ = v3;

    RenderCommand cmd;
    cmd.material = s_DefaultMaterial;
    cmd.transform = transform;
    cmd.vertexCount = 6;
    cmd.vertexOffset = s_VertexCount;
    cmd.texture = texture;

    s_Commands.push_back(cmd);
    s_VertexCount += 4;
}

void Renderer2D::DrawCircle(const glm::vec3& position, float radius, const glm::vec4& color, int32_t depth) {
    // For now, render as a quad (proper circle shader can be added later)
    DrawQuad(position, {radius * 2.0f, radius * 2.0f}, color, depth);
}

void Renderer2D::DrawLineStrip(const std::vector<glm::vec3>& points, const glm::vec4& color, float thickness, int32_t depth) {
    if (points.size() < 2) return;

    const float half = thickness * 0.5f;
    size_t n = points.size();

    for (size_t i = 0; i < n - 1; ++i) {
        if (s_VertexCount + 4 > MAX_VERTICES) {
            AXIOM_WARN("Vertex buffer full!");
            return;
        }

        glm::vec3 p0 = points[i];
        glm::vec3 p1 = points[i + 1];

        glm::vec2 dir = glm::normalize(glm::vec2(p1 - p0));
        glm::vec2 normal = glm::vec2(-dir.y, dir.x);
        glm::vec2 offset = normal * half;

        float zOffset = depth * 0.001f;

        // Create line quad vertices
        Vertex v0 = {{p0.x - offset.x, p0.y - offset.y, p0.z + zOffset}, {0.0f, 0.0f}, color, -1.0f};
        Vertex v1 = {{p1.x - offset.x, p1.y - offset.y, p1.z + zOffset}, {1.0f, 0.0f}, color, -1.0f};
        Vertex v2 = {{p1.x + offset.x, p1.y + offset.y, p1.z + zOffset}, {1.0f, 1.0f}, color, -1.0f};
        Vertex v3 = {{p0.x + offset.x, p0.y + offset.y, p0.z + zOffset}, {0.0f, 1.0f}, color, -1.0f};

        *s_VertexPtr++ = v0;
        *s_VertexPtr++ = v1;
        *s_VertexPtr++ = v2;
        *s_VertexPtr++ = v3;

        RenderCommand cmd;
        cmd.material = s_DefaultMaterial;
        cmd.transform = glm::mat4(1.0f);
        cmd.vertexCount = 6;
        cmd.vertexOffset = s_VertexCount;
        cmd.texture = nullptr;

        s_Commands.push_back(cmd);
        s_VertexCount += 4;
    }
}

std::shared_ptr<Shader> Renderer2D::GetQuadShader() {
    return s_QuadShader;
}

}
