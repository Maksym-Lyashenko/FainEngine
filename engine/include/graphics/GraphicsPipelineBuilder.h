#pragma once

#include "vk/VulkanContext.h"
#include "vk/VulkanResourceHolder.h"

#include <span>

namespace eng
{
class GraphicsPipelineBuilder
{
 public:
  explicit GraphicsPipelineBuilder(IContext& ctx) : m_ctx(ctx)
  {
    m_desc.colorFormats[0] = ctx.getSwapchainFormat();
  }

  GraphicsPipelineBuilder& Shaders(ShaderModuleHandle vert, ShaderModuleHandle frag)
  {
    m_desc.smVert = vert;
    m_desc.smFrag = frag;
    return *this;
  }

  GraphicsPipelineBuilder& ColorFormat(VkFormat format)
  {
    m_desc.colorFormats[0] = format;
    return *this;
  }

  GraphicsPipelineBuilder& VertexLayout(
      std::span<const VkVertexInputBindingDescription> bindings,
      std::span<const VkVertexInputAttributeDescription>
          attributes)
  {
    m_desc.vertexBindings = bindings;
    m_desc.vertexAttributes = attributes;
    return *this;
  }

  GraphicsPipelineBuilder& Specialization(
      VkShaderStageFlagBits stage,
      std::span<const VkSpecializationMapEntry>
          entries,
      const void* data,
      size_t dataSize)
  {
    m_desc.specialization.stage = stage;
    m_desc.specialization.entries = entries;
    m_desc.specialization.data = data;
    m_desc.specialization.dataSize = dataSize;
    return *this;
  }

  GraphicsPipelineBuilder& PushConstants(uint32_t size, VkShaderStageFlags stages)
  {
    m_desc.pushConstantSize = size;
    m_desc.pushConstantStages = stages;
    return *this;
  }

  GraphicsPipelineBuilder& Depth(
      VkFormat depthFormat,
      bool depthTestEnable = true,
      bool depthWriteEnable = true,
      VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS)
  {
    m_desc.depthFormat = depthFormat;
    m_desc.depthTestEnable = depthTestEnable;
    m_desc.depthWriteEnable = depthWriteEnable;
    m_desc.depthCompareOp = depthCompareOp;
    return *this;
  }

  GraphicsPipelineBuilder& DepthBias(bool enable = true)
  {
    m_desc.depthBiasEnable = enable;
    return *this;
  }

  GraphicsPipelineBuilder& Topology(VkPrimitiveTopology topology)
  {
    m_desc.topology = topology;
    return *this;
  }

  GraphicsPipelineBuilder& FillMode(VkPolygonMode mode)
  {
    m_desc.fillMode = mode;
    return *this;
  }

  GraphicsPipelineBuilder& CullMode(VkCullModeFlags mode)
  {
    m_desc.cullMode = mode;
    return *this;
  }

  GraphicsPipelineBuilder& FrontFace(VkFrontFace face)
  {
    m_desc.frontFace = face;
    return *this;
  }

  GraphicsPipelineBuilder& CullBack()
  {
    m_desc.cullMode = VK_CULL_MODE_BACK_BIT;
    return *this;
  }

  GraphicsPipelineBuilder& NoCulling()
  {
    m_desc.cullMode = VK_CULL_MODE_NONE;
    return *this;
  }

  Holder<RenderPipelineHandle> Build()
  {
    return Holder<RenderPipelineHandle>(&m_ctx, m_ctx.createRenderPipeline(m_desc));
  }

  const RenderPipelineDesc& Desc() const { return m_desc; }

  template <typename TVertex>
  GraphicsPipelineBuilder& VertexLayout()
  {
    static constexpr auto bindings = TVertex::Bindings();
    static constexpr auto attributes = TVertex::Attributes();

    m_desc.vertexBindings = bindings;
    m_desc.vertexAttributes = attributes;
    return *this;
  }

 private:
  IContext& m_ctx;
  RenderPipelineDesc m_desc{};
};

}  // namespace eng