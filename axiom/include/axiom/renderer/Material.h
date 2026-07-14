#pragma once
#include <axiom/renderer/rhi/BindGroup.h>
#include <axiom/renderer/rhi/PipelineDesc.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace axiom::renderer {

enum class ShaderID : uint64_t { Invalid = 0 };

// Ein Parameter kann direktes Binding ODER Bindless-Index sein (Punkt 14).
struct ResourceBinding {
    enum class Kind { Direct, BindlessIndex } kind = Kind::Direct;
    rhi::BufferHandle buffer;
    rhi::TextureHandle texture;
    uint32_t bindlessIndex = 0; // gültig wenn kind == BindlessIndex
};

class Material {
  public:
    ShaderID shaderId = ShaderID::Invalid;
    std::unordered_map<std::string, ResourceBinding> params;
    rhi::PipelineState state;

    [[nodiscard]] rhi::PipelineHandle pipelineHandle() const {
        return m_pipeline;
    }
    [[nodiscard]] rhi::BindGroupHandle bindGroupHandle() const {
        return m_bindGroup;
    }

  private:
    rhi::PipelineHandle m_pipeline;
    rhi::BindGroupHandle m_bindGroup;
    friend class MaterialSystem; // baut Pipeline/BindGroup aus shaderId+params
};

} // namespace axiom::renderer