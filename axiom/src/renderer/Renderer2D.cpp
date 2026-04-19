// Renderer2D.cpp
#include "axiom/renderer/Renderer2D.h"
#include "axiom/renderer/Renderer.h"
#include "axiom/assets/AssetManager.h"
#include "axiom/renderer/Texture2D.h"
#include "axiom/renderer/Material.h"
#include "axiom/renderer/Shader.h"
#include "axiom/renderer/Sprite.h"
#include "axiom/renderer/VertexArray.h"
#include "axiom/renderer/VertexBuffer.h"
#include "axiom/renderer/IndexBuffer.h"
#include <vector>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include "axiom/renderer/IndirectDrawBuffer.h"

namespace axiom {

struct QuadDrawCommand {
    glm::mat4 transform;
    glm::vec4 color;
    glm::vec2 texCoords[4];
    float texIndex;
    float tiling;
    float z;
};

static std::vector<QuadDrawCommand> s_QuadBatch;

Renderer2D::RendererData* Renderer2D::s_Data = nullptr;

namespace {
	constexpr float kEpsilon = 0.0001f;

	std::shared_ptr<Material> CreateImmediateColorMaterial() {
		static const char* source = R"(#type vertex
#version 460 core

layout(location = 0) in vec4 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in float a_TilingFactor;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec4 v_Color;

void main()
{
    v_Color = a_Color;
    gl_Position = u_ViewProjection * u_Transform * a_Position;
}

#type fragment
#version 460 core

layout(location = 0) out vec4 FragColor;

in vec4 v_Color;

void main()
{
    FragColor = v_Color;
}
)";

			return std::make_shared<Material>(Shader::CreateFromMemory(source));
		}

		std::shared_ptr<Material> CreateImmediateQuadMaterial() {
			static const char* source = R"(#type vertex
#version 460 core

layout(location = 0) in vec4 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in float a_TilingFactor;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec4 v_Color;
out vec2 v_TexCoord;
out float v_TilingFactor;
out float v_TexIndex;

void main()
{
    v_Color = a_Color;
    v_TexCoord = a_TexCoord;
    v_TilingFactor = a_TilingFactor;
    v_TexIndex = a_TexIndex;
    gl_Position = u_ViewProjection * u_Transform * a_Position;
}

#type fragment
#version 460 core

layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in vec2 v_TexCoord;
in float v_TilingFactor;
in float v_TexIndex;

uniform sampler2D u_Textures[32];

void main()
{
    int index = int(v_TexIndex);
    vec4 tex = texture(u_Textures[index], v_TexCoord * v_TilingFactor);
    FragColor = tex * v_Color;
}
)";

			return std::make_shared<Material>(Shader::CreateFromMemory(source));
		}
	}

	void Renderer2D::Init() {
		s_Data = new RendererData();

		auto whiteTexture = Texture2D::Create(1, 1, false,
											  TextureWrap::ClampToEdge, TextureWrap::ClampToEdge,
											  TextureFilter::Linear, TextureFilter::Linear);
		uint32_t white = 0xffffffff;
		whiteTexture->SetData(&white, 1, 1, TextureFormat::RGBA8);

		s_Data->TextureSlots[0] = whiteTexture;
		s_Data->TextureSlotIndex = 1;

		// Quad
		s_Data->QuadVAO = VertexArray::Create();
		s_Data->QuadVBO = VertexBuffer::Create(
			RendererData::MaxVertices * sizeof(QuadVertex),
			BufferUsage::Dynamic
		);
		s_Data->QuadVBO->SetLayout(BufferLayout(sizeof(QuadVertex), {
			{ ShaderDataType::Vec4, "a_Position", static_cast<uint32_t>(offsetof(QuadVertex, Position)), false },
			{ ShaderDataType::Vec4, "a_Color", static_cast<uint32_t>(offsetof(QuadVertex, Color)), false },
			{ ShaderDataType::Vec2, "a_TexCoord", static_cast<uint32_t>(offsetof(QuadVertex, TexCoord)), false },
			{ ShaderDataType::Float, "a_TexIndex", static_cast<uint32_t>(offsetof(QuadVertex, TexIndex)), false },
			{ ShaderDataType::Float, "a_TilingFactor", static_cast<uint32_t>(offsetof(QuadVertex, TilingFactor)), false }
								   }));
		s_Data->QuadVAO->AddVertexBuffer(s_Data->QuadVBO);

		uint32_t* quadIndices = new uint32_t[RendererData::MaxIndices];
		uint32_t quadOffset = 0;
		for (uint32_t i = 0; i < RendererData::MaxIndices; i += 6) {
			quadIndices[i + 0] = quadOffset + 0;
			quadIndices[i + 1] = quadOffset + 1;
			quadIndices[i + 2] = quadOffset + 2;
			quadIndices[i + 3] = quadOffset + 2;
			quadIndices[i + 4] = quadOffset + 3;
			quadIndices[i + 5] = quadOffset + 0;
			quadOffset += 4;
		}
		s_Data->QuadIBO = IndexBuffer::Create(quadIndices, RendererData::MaxIndices);
		s_Data->QuadVAO->SetIndexBuffer(s_Data->QuadIBO);
		delete[] quadIndices;
		s_Data->QuadBufferBase = new QuadVertex[RendererData::MaxVertices];

		// Circle
		s_Data->CircleVAO = VertexArray::Create();
		s_Data->CircleVBO = VertexBuffer::Create(
			RendererData::MaxVertices * sizeof(CircleVertex),
			BufferUsage::Dynamic
		);
		s_Data->CircleVBO->SetLayout(BufferLayout(sizeof(CircleVertex), {
			{ ShaderDataType::Vec3, "a_Position", static_cast<uint32_t>(offsetof(CircleVertex, Position)), false },
			{ ShaderDataType::Vec3, "a_LocalPosition", static_cast<uint32_t>(offsetof(CircleVertex, LocalPosition)), false },
			{ ShaderDataType::Vec4, "a_Color", static_cast<uint32_t>(offsetof(CircleVertex, Color)), false },
			{ ShaderDataType::Float, "a_Thickness", static_cast<uint32_t>(offsetof(CircleVertex, Thickness)), false }
									 }));
		s_Data->CircleVAO->AddVertexBuffer(s_Data->CircleVBO);
		s_Data->CircleVAO->SetIndexBuffer(s_Data->QuadIBO);
		s_Data->CircleBufferBase = new CircleVertex[RendererData::MaxVertices];

		// Skinned
		s_Data->SkinnedVAO = VertexArray::Create();
		s_Data->SkinnedVBO = VertexBuffer::Create(
			RendererData::MaxVertices * sizeof(SkinnedVertex2D),
			BufferUsage::Dynamic
		);
		s_Data->SkinnedVBO->SetLayout(BufferLayout(sizeof(SkinnedVertex2D), {
			{ ShaderDataType::Vec3, "a_Position", static_cast<uint32_t>(offsetof(SkinnedVertex2D, Position)), false },
			{ ShaderDataType::Vec4, "a_Color", static_cast<uint32_t>(offsetof(SkinnedVertex2D, Color)), false },
			{ ShaderDataType::Vec2, "a_TexCoord", static_cast<uint32_t>(offsetof(SkinnedVertex2D, TexCoord)), false },
			{ ShaderDataType::Vec4, "a_BoneIndices", static_cast<uint32_t>(offsetof(SkinnedVertex2D, BoneIndices)), false },
			{ ShaderDataType::Vec4, "a_BoneWeights", static_cast<uint32_t>(offsetof(SkinnedVertex2D, BoneWeights)), false }
									  }));
		s_Data->SkinnedVAO->AddVertexBuffer(s_Data->SkinnedVBO);

		auto quadShader = AssetManager::Get<Shader>("engine://shaders/Renderer2D/Quad.glsl");
		auto circleShader = AssetManager::Get<Shader>("engine://shaders/Renderer2D/Circle.glsl");
		auto skinnedShader = AssetManager::Get<Shader>("engine://shaders/Renderer2D/Skinned.glsl");

		s_Data->QuadMaterial = std::make_shared<Material>(quadShader);
		s_Data->CircleMaterial = std::make_shared<Material>(circleShader);
		s_Data->SkinnedMaterial = std::make_shared<Material>(skinnedShader);

		Configure2DMaterial(s_Data->QuadMaterial);
		Configure2DMaterial(s_Data->CircleMaterial);
		Configure2DMaterial(s_Data->SkinnedMaterial);

		quadShader->Bind();
		for (uint32_t i = 0; i < RendererData::MaxTextureSlots; i++) {
			quadShader->SetUniform1i("u_Textures[" + std::to_string(i) + "]", static_cast<int>(i));
		}

		s_Data->QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		s_Data->QuadVertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
		s_Data->QuadVertexPositions[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
		s_Data->QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

		StartBatch();
	}

	void Renderer2D::Shutdown() {
		delete[] s_Data->QuadBufferBase;
		delete[] s_Data->CircleBufferBase;
		delete s_Data;
		s_Data = nullptr;
	}

	void Renderer2D::BeginScene() {
		StartBatch();
	}

	void Renderer2D::EndScene() {
		FlushAll();
	}

	void Renderer2D::StartBatch() {
		s_Data->QuadBufferPtr = s_Data->QuadBufferBase;
		s_Data->CircleBufferPtr = s_Data->CircleBufferBase;
		s_Data->TextureSlotIndex = 1;
	}

	void Renderer2D::FlushAll() {
		FlushQuads();
		FlushCircles();
		StartBatch();
	}

	void Renderer2D::EnsureQuadCapacity(uint32_t quadCount) {
		const uint32_t usedVertices = static_cast<uint32_t>(s_Data->QuadBufferPtr - s_Data->QuadBufferBase);
		if (usedVertices + quadCount * 4 > RendererData::MaxVertices) {
			FlushAll();
		}
	}

	void Renderer2D::EnsureCircleCapacity(uint32_t circleCount) {
		const uint32_t usedVertices = static_cast<uint32_t>(s_Data->CircleBufferPtr - s_Data->CircleBufferBase);
		if (usedVertices + circleCount * 4 > RendererData::MaxVertices) {
			FlushAll();
		}
	}

	float Renderer2D::AcquireTextureSlot(const std::shared_ptr<Texture2D>& texture) {
		if (!texture) {
			return 0.0f;
		}

		for (uint32_t i = 1; i < s_Data->TextureSlotIndex; i++) {
			if (s_Data->TextureSlots[i].get() == texture.get()) {
				return static_cast<float>(i);
			}
		}

		if (s_Data->TextureSlotIndex >= RendererData::MaxTextureSlots) {
			FlushAll();
		}

		const uint32_t slot = s_Data->TextureSlotIndex;
		s_Data->TextureSlots[slot] = texture;
		s_Data->TextureSlotIndex++;
		return static_cast<float>(slot);
	}

	void Renderer2D::Configure2DMaterial(const std::shared_ptr<Material>& material) {
		if (!material) {
			return;
		}

		auto& state = material->GetRenderState();
		state.DepthTest = true;
		state.DepthFunction = DepthFunc::LessEqual;
		state.DepthWrite = true;
		state.Blending = true;
		state.BlendSrc = BlendFactor::SrcAlpha;
		state.BlendDst = BlendFactor::OneMinusSrcAlpha;
		state.CullFace = false;
	}

	void Renderer2D::FlushQuads() {
		const uint32_t quadCount = static_cast<uint32_t>(s_QuadBatch.size());
		if (quadCount == 0) return;

		// Z-Sort
		std::sort(s_QuadBatch.begin(), s_QuadBatch.end(), [](const QuadDrawCommand& a, const QuadDrawCommand& b) {
			return a.z < b.z;
		});

		// Schreibe Vertices in Buffer
		QuadVertex* vtx = s_Data->QuadBufferBase;
		for (const auto& cmd : s_QuadBatch) {
			for (int i = 0; i < 4; ++i) {
				vtx->Position = cmd.transform * s_Data->QuadVertexPositions[i];
				vtx->Color = cmd.color;
				vtx->TexCoord = cmd.texCoords[i];
				vtx->TexIndex = cmd.texIndex;
				vtx->TilingFactor = cmd.tiling;
				++vtx;
			}
		}

		const uint32_t vertexCount = quadCount * 4;
		const uint32_t indexCount = quadCount * 6;
		s_Data->QuadVBO->SetData(s_Data->QuadBufferBase, vertexCount * sizeof(QuadVertex));

		for (uint32_t i = 0; i < s_Data->TextureSlotIndex; i++) {
			if (s_Data->TextureSlots[i]) {
				s_Data->TextureSlots[i]->Bind(static_cast<int>(i));
			}
		}

		auto material = s_Data->QuadMaterial;
		Configure2DMaterial(material);
		Renderer::Submit(s_Data->QuadVAO, material, indexCount, glm::mat4(1.0f));

		s_QuadBatch.clear();
	}

	void Renderer2D::FlushCircles() {
		const uint32_t vertexCount = static_cast<uint32_t>(s_Data->CircleBufferPtr - s_Data->CircleBufferBase);
		if (vertexCount == 0) {
			return;
		}

		const uint32_t size = vertexCount * sizeof(CircleVertex);
		s_Data->CircleVBO->SetData(s_Data->CircleBufferBase, size);

		auto material = s_Data->CircleMaterial;
		Configure2DMaterial(material);

		const uint32_t indexCount = (vertexCount / 4) * 6;
		Renderer::Submit(s_Data->CircleVAO, material, indexCount, glm::mat4(1.0f));
	}

	// --- Quads ---

	void Renderer2D::DrawQuad(const glm::vec2& pos,
							  const glm::vec2& size,
							  const glm::vec4& color,
							  float z) {
		DrawQuad(glm::vec3(pos, z), size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& pos,
							  const glm::vec2& size,
							  const glm::vec4& color) {
		glm::mat4 transform =
			glm::translate(glm::mat4(1.0f), pos) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, color);
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform,
							  const glm::vec4& color) {
		static const glm::vec2 texCoords[4] = {
			{ 0.0f, 0.0f },
			{ 1.0f, 0.0f },
			{ 1.0f, 1.0f },
			{ 0.0f, 1.0f }
		};
		QuadDrawCommand cmd;
		cmd.transform = transform;
		cmd.color = color;
		for (int i = 0; i < 4; ++i) cmd.texCoords[i] = texCoords[i];
		cmd.texIndex = 0.0f;
		cmd.tiling = 1.0f;
		cmd.z = transform[3].z;
		s_QuadBatch.push_back(cmd);
	}

	void Renderer2D::DrawQuad(const glm::vec2& pos,
							  const glm::vec2& size,
							  const std::shared_ptr<Texture2D>& texture,
							  float tiling,
							  const glm::vec4& tint,
							  float z) {
		DrawQuad(glm::vec3(pos, z), size, texture, tiling, tint);
	}

	void Renderer2D::DrawQuad(const glm::vec3& pos,
							  const glm::vec2& size,
							  const std::shared_ptr<Texture2D>& texture,
							  float tiling,
							  const glm::vec4& tint) {
		glm::mat4 transform =
			glm::translate(glm::mat4(1.0f), pos) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, texture, tiling, tint);
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform,
							  const std::shared_ptr<Texture2D>& texture,
							  float tiling,
							  const glm::vec4& tint) {
		static const glm::vec2 texCoords[4] = {
			{ 0.0f, 0.0f },
			{ 1.0f, 0.0f },
			{ 1.0f, 1.0f },
			{ 0.0f, 1.0f }
		};
		QuadDrawCommand cmd;
		cmd.transform = transform;
		cmd.color = tint;
		for (int i = 0; i < 4; ++i) cmd.texCoords[i] = texCoords[i];
		cmd.texIndex = AcquireTextureSlot(texture);
		cmd.tiling = tiling;
		cmd.z = transform[3].z;
		s_QuadBatch.push_back(cmd);
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform,
							  const std::shared_ptr<Texture2D>& texture,
							  const std::shared_ptr<Material>& materialOverride,
							  float tiling,
							  const glm::vec4& tint) {
		if (!materialOverride) {
			DrawQuad(transform, texture, tiling, tint);
			return;
		}

		FlushAll();

		static const glm::vec2 texCoords[4] = {
			{ 0.0f, 0.0f },
			{ 1.0f, 0.0f },
			{ 1.0f, 1.0f },
			{ 0.0f, 1.0f }
		};

		QuadVertex quadVertices[4];
		const float texIndex = 0.0f;
		for (int i = 0; i < 4; i++) {
			quadVertices[i].Position = transform * s_Data->QuadVertexPositions[i];
			quadVertices[i].Color = tint;
			quadVertices[i].TexCoord = texCoords[i];
			quadVertices[i].TexIndex = texIndex;
			quadVertices[i].TilingFactor = tiling;
		}

		s_Data->QuadVBO->SetData(quadVertices, sizeof(quadVertices));

		Configure2DMaterial(materialOverride);
		if (texture && materialOverride->GetShader() && materialOverride->GetShader()->HasUniform("u_Texture")) {
			materialOverride->SetTexture("u_Texture", texture);
		}

		Renderer::Submit(s_Data->QuadVAO, materialOverride, 6, glm::mat4(1.0f));
	}

	// --- Sprites ---

	void Renderer2D::DrawSprite(const glm::mat4& transform,
								const Sprite& sprite,
								const glm::vec4& tint) {
		const glm::vec2 uv[4] = {
			{ sprite.UVMin.x, sprite.UVMin.y },
			{ sprite.UVMax.x, sprite.UVMin.y },
			{ sprite.UVMax.x, sprite.UVMax.y },
			{ sprite.UVMin.x, sprite.UVMax.y }
		};

		EnsureQuadCapacity();

		const float texIndex = AcquireTextureSlot(sprite.Texture);
		const float tiling = 1.0f;

		for (int i = 0; i < 4; i++) {
			s_Data->QuadBufferPtr->Position = transform * s_Data->QuadVertexPositions[i];
			s_Data->QuadBufferPtr->Color = tint;
			s_Data->QuadBufferPtr->TexCoord = uv[i];
			s_Data->QuadBufferPtr->TexIndex = texIndex;
			s_Data->QuadBufferPtr->TilingFactor = tiling;
			s_Data->QuadBufferPtr++;
		}
	}

	void Renderer2D::DrawSprite(const glm::mat4& transform,
								const Sprite& sprite,
								const glm::vec4& tint,
								const std::shared_ptr<Material>& materialOverride) {
		if (!materialOverride) {
			DrawSprite(transform, sprite, tint);
			return;
		}

		FlushAll();

		const glm::vec2 uv[4] = {
			{ sprite.UVMin.x, sprite.UVMin.y },
			{ sprite.UVMax.x, sprite.UVMin.y },
			{ sprite.UVMax.x, sprite.UVMax.y },
			{ sprite.UVMin.x, sprite.UVMax.y }
		};

		QuadVertex quadVertices[4];
		const float texIndex = 0.0f;
		const float tiling = 1.0f;

		for (int i = 0; i < 4; i++) {
			quadVertices[i].Position = transform * s_Data->QuadVertexPositions[i];
			quadVertices[i].Color = tint;
			quadVertices[i].TexCoord = uv[i];
			quadVertices[i].TexIndex = texIndex;
			quadVertices[i].TilingFactor = tiling;
		}

		s_Data->QuadVBO->SetData(quadVertices, sizeof(quadVertices));

		Configure2DMaterial(materialOverride);
		if (sprite.Texture && materialOverride->GetShader() && materialOverride->GetShader()->HasUniform("u_Texture")) {
			materialOverride->SetTexture("u_Texture", sprite.Texture);
		}

		Renderer::Submit(s_Data->QuadVAO, materialOverride, 6, glm::mat4(1.0f));

	}

	// --- Skinned ---

	void Renderer2D::DrawSkinned(const glm::mat4& transform,
								 const SkinnedMesh2D& mesh,
								 const SkeletonPose2D& pose,
								 const glm::vec4& tint,
								 const std::shared_ptr<Material>& materialOverride) {
		if (mesh.Vertices.empty() || mesh.Indices.empty()) {
			return;
		}

		if (mesh.Vertices.size() > RendererData::MaxVertices || mesh.Indices.size() > RendererData::MaxIndices) {
			return;
		}

		FlushAll();

		std::vector<SkinnedVertex2D> vertices = mesh.Vertices;
		for (auto& vertex : vertices) {
			vertex.Color *= tint;
		}

		s_Data->SkinnedVBO->SetData(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(SkinnedVertex2D)));
		s_Data->SkinnedIBO = IndexBuffer::Create(const_cast<uint32_t*>(mesh.Indices.data()), static_cast<uint32_t>(mesh.Indices.size()));
		s_Data->SkinnedVAO->SetIndexBuffer(s_Data->SkinnedIBO);

		auto material = materialOverride ? materialOverride : s_Data->SkinnedMaterial;
		Configure2DMaterial(material);

		auto shader = material->GetShader();
		shader->Bind();

		const uint32_t boneCount = pose.BoneCount < MaxBones ? pose.BoneCount : MaxBones;
		for (uint32_t i = 0; i < boneCount; i++) {
			shader->SetUniformMat4("u_BoneTransforms[" + std::to_string(i) + "]", pose.BoneTransforms[i]);
		}

		if (mesh.Texture && shader->HasUniform("u_Texture")) {
			mesh.Texture->Bind(0);
			shader->SetUniform1i("u_Texture", 0);
		}

		Renderer::Submit(s_Data->SkinnedVAO, material, s_Data->SkinnedIBO->GetCount(), transform);
	}

	// --- Lines / Rects / Circles ---

	void Renderer2D::DrawLine(const glm::vec3& p0,
							  const glm::vec3& p1,
							  const glm::vec4& color) {
		DrawLine(p0, p1, color, 1.5f);
	}

	void Renderer2D::DrawLine(const glm::vec3& p0,
							  const glm::vec3& p1,
							  const glm::vec4& color,
							  float thickness) {
		DrawLine(p0, p1, color, thickness, LineCap::Butt);
	}

	void Renderer2D::DrawLine(const glm::vec3& p0,
							  const glm::vec3& p1,
							  const glm::vec4& color,
							  float thickness,
							  LineCap cap) {
		if (thickness <= 0.0f) {
			return;
		}

		const glm::vec2 delta = glm::vec2(p1) - glm::vec2(p0);
		const float length = glm::length(delta);
		if (length <= kEpsilon) {
			DrawCircle(p0, thickness * 0.5f, 0.0f, color);
			return;
		}

		glm::vec3 start = p0;
		glm::vec3 end = p1;
		const glm::vec2 dir = delta / length;
		const float halfThickness = thickness * 0.5f;

		if (cap == LineCap::Square) {
			start -= glm::vec3(dir * halfThickness, 0.0f);
			end += glm::vec3(dir * halfThickness, 0.0f);
		}

		const glm::vec2 finalDelta = glm::vec2(end) - glm::vec2(start);
		const float finalLength = glm::length(finalDelta);
		const glm::vec3 center = (start + end) * 0.5f;
		const float angle = std::atan2(finalDelta.y, finalDelta.x);

		glm::mat4 transform =
			glm::translate(glm::mat4(1.0f), center) *
			glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f)) *
			glm::scale(glm::mat4(1.0f), glm::vec3(finalLength, thickness, 1.0f));

		DrawQuad(transform, color);

		if (cap == LineCap::Round) {
			DrawCircle(p0, halfThickness, 0.0f, color);
			DrawCircle(p1, halfThickness, 0.0f, color);
		}
	}

	void Renderer2D::DrawLineStrip(const std::vector<glm::vec3>& points,
								   const glm::vec4& color,
								   float thickness,
								   bool closed,
								   LineCap cap,
								   LineJoin join,
								   float miterLimit) {
		if (points.size() < 2 || thickness <= 0.0f) {
			return;
		}

		const size_t pointCount = points.size();
		const size_t segmentCount = closed ? pointCount : pointCount - 1;
		const LineCap segmentCap = closed ? LineCap::Butt : cap;

		for (size_t i = 0; i < segmentCount; i++) {
			const glm::vec3& a = points[i];
			const glm::vec3& b = points[(i + 1) % pointCount];
			DrawLine(a, b, color, thickness, segmentCap);

			if (i + 1 < pointCount || closed) {
				const glm::vec3& c = points[(i + 2) % pointCount];
				glm::vec2 v1 = glm::normalize(glm::vec2(b) - glm::vec2(a));
				glm::vec2 v2 = glm::normalize(glm::vec2(c) - glm::vec2(b));
				float dot = glm::clamp(glm::dot(v1, v2), -1.0f, 1.0f);
				float angle = std::acos(dot);

				switch (join) {
				case LineJoin::Round:
					DrawCircle(b, thickness * 0.5f, 0.0f, color);
					break;
				case LineJoin::Bevel:
					// Bevel: nothing extra, segments already overlap visually
					break;
				case LineJoin::Miter:
					if (angle < glm::pi<float>() * 0.5f && miterLimit > 0.0f) {
						// Simple miter: small circle hint
						DrawCircle(b, thickness * 0.35f, 0.0f, color);
					}
					break;
				}
			}
		}
	}

	void Renderer2D::DrawRect(const glm::vec3& pos,
							  const glm::vec2& size,
							  const glm::vec4& color,
							  float z,
							  float thickness) {
		glm::vec3 p0 = pos + glm::vec3(0.0f, 0.0f, z);
		glm::vec3 p1 = pos + glm::vec3(size.x, 0.0f, z);
		glm::vec3 p2 = pos + glm::vec3(size.x, size.y, z);
		glm::vec3 p3 = pos + glm::vec3(0.0f, size.y, z);

		DrawLine(p0, p1, color, thickness, LineCap::Butt);
		DrawLine(p1, p2, color, thickness, LineCap::Butt);
		DrawLine(p2, p3, color, thickness, LineCap::Butt);
		DrawLine(p3, p0, color, thickness, LineCap::Butt);
	}

	void Renderer2D::DrawRect(const glm::mat4& transform,
							  const glm::vec4& color,
							  float thickness) {
		glm::vec3 points[4];
		for (int i = 0; i < 4; i++) {
			points[i] = transform * s_Data->QuadVertexPositions[i];
		}

		DrawLine(points[0], points[1], color, thickness, LineCap::Butt);
		DrawLine(points[1], points[2], color, thickness, LineCap::Butt);
		DrawLine(points[2], points[3], color, thickness, LineCap::Butt);
		DrawLine(points[3], points[0], color, thickness, LineCap::Butt);
	}

	void Renderer2D::DrawCircle(const glm::vec2& pos,
								float radius,
								float thickness,
								const glm::vec4& color,
								float z) {
		DrawCircle(glm::vec3(pos, z), radius, thickness, color);
	}

	void Renderer2D::DrawCircle(const glm::vec3& pos,
								float radius,
								float thickness,
								const glm::vec4& color) {
		glm::mat4 transform =
			glm::translate(glm::mat4(1.0f), pos) *
			glm::scale(glm::mat4(1.0f), glm::vec3(radius * 2.0f, radius * 2.0f, 1.0f));

		DrawCircle(transform, thickness, color);
	}

	void Renderer2D::DrawCircle(const glm::mat4& transform,
								float thickness,
								const glm::vec4& color) {
		EnsureCircleCapacity();

		for (int i = 0; i < 4; i++) {
			s_Data->CircleBufferPtr->Position = transform * s_Data->QuadVertexPositions[i];
			s_Data->CircleBufferPtr->LocalPosition = glm::vec3(glm::vec2(s_Data->QuadVertexPositions[i]) * 2.0f, 0.0f);
			s_Data->CircleBufferPtr->Color = color;
			s_Data->CircleBufferPtr->Thickness = thickness;
			s_Data->CircleBufferPtr++;
		}
	}

} // namespace axiom
