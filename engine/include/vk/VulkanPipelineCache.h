#pragma once

#include "vk/VulkanCommon.h"

#include <array>
#include <cstddef>
#include <vector>
#include <span>

namespace eng
{
struct ShaderModuleDesc
{
  const uint32_t* code = nullptr;
  size_t sizeByte = 0;
};

struct PipelineSpecializationDesc
{
  VkShaderStageFlagBits stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  std::span<const VkSpecializationMapEntry> entries{};
  const void* data = nullptr;
  size_t dataSize = 0;
};

struct RenderPipelineDesc
{
  ShaderModuleHandle smVert{};
  ShaderModuleHandle smFrag{};

  std::array<VkFormat, 1> colorFormats{VK_FORMAT_B8G8R8A8_UNORM};

  std::span<const VkVertexInputBindingDescription> vertexBindings{};
  std::span<const VkVertexInputAttributeDescription> vertexAttributes{};

  PipelineSpecializationDesc specialization{};

  std::span<const VkDescriptorSetLayout> descriptorSetLayouts{};

  uint32_t pushConstantSize = 0;
  VkShaderStageFlags pushConstantStages = 0;

  VkFormat depthFormat = VK_FORMAT_UNDEFINED;
  bool depthTestEnable = false;
  bool depthWriteEnable = false;
  VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS;

  VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  VkPolygonMode fillMode = VK_POLYGON_MODE_FILL;
  VkCullModeFlags cullMode = VK_CULL_MODE_NONE;
  VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

  bool depthBiasEnable = false;
};

class VulkanPipelineCache
{
 public:
  VulkanPipelineCache() = default;
  ~VulkanPipelineCache();

  VulkanPipelineCache(const VulkanPipelineCache&) = delete;
  VulkanPipelineCache& operator=(const VulkanPipelineCache&) = delete;

  void create(VkDevice device);
  void destroy();

  ShaderModuleHandle createShaderModule(const ShaderModuleDesc& desc);
  void destroyShaderModule(ShaderModuleHandle handle);

  RenderPipelineHandle createRenderPipeline(const RenderPipelineDesc& desc);
  void destroyRenderPipeline(RenderPipelineHandle handle);

  VkPipeline getPipeline(RenderPipelineHandle handle) const;
  VkPipelineLayout getPipelineLayout(RenderPipelineHandle handle) const;

 private:
  struct PipelineData
  {
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
  };

 private:
  VkDevice m_device = VK_NULL_HANDLE;
  std::vector<VkShaderModule> m_shaderModules;
  std::vector<PipelineData> m_pipelines;
};

}  // namespace eng