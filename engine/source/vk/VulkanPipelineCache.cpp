#include "vk/VulkanPipelineCache.h"

#include <array>
#include <stdexcept>

namespace eng
{
namespace
{
void vkCheck(VkResult result, const char* what)
{
  if (result != VK_SUCCESS)
  {
    throw std::runtime_error(std::string("Vulkan error at: ") + what);
  }
}
}  // namespace

VulkanPipelineCache::~VulkanPipelineCache()
{
  destroy();
}

void VulkanPipelineCache::create(VkDevice device)
{
  m_device = device;
}

void VulkanPipelineCache::destroy()
{
  for (auto& p : m_pipelines)
  {
    if (p.pipeline != VK_NULL_HANDLE)
    {
      vkDestroyPipeline(m_device, p.pipeline, nullptr);
      p.pipeline = VK_NULL_HANDLE;
    }

    if (p.layout != VK_NULL_HANDLE)
    {
      vkDestroyPipelineLayout(m_device, p.layout, nullptr);
      p.layout = VK_NULL_HANDLE;
    }
  }
  m_pipelines.clear();

  for (VkShaderModule sm : m_shaderModules)
  {
    if (sm != VK_NULL_HANDLE)
    {
      vkDestroyShaderModule(m_device, sm, nullptr);
    }
  }
  m_shaderModules.clear();

  m_device = VK_NULL_HANDLE;
}

ShaderModuleHandle VulkanPipelineCache::createShaderModule(const ShaderModuleDesc& desc)
{
  if (desc.code == nullptr || desc.sizeByte == 0)
  {
    throw std::runtime_error("createShaderModule(): empty shader code");
  }

  VkShaderModuleCreateInfo ci{};
  ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  ci.codeSize = desc.sizeByte;
  ci.pCode = desc.code;

  VkShaderModule shader = VK_NULL_HANDLE;
  vkCheck(vkCreateShaderModule(m_device, &ci, nullptr, &shader), "vkCreateShaderModule");

  m_shaderModules.push_back(shader);
  return ShaderModuleHandle{static_cast<uint32_t>(m_shaderModules.size() - 1)};
}

void VulkanPipelineCache::destroyShaderModule(ShaderModuleHandle handle)
{
  if (handle.id >= m_shaderModules.size())
  {
    return;
  }

  if (m_shaderModules[handle.id] == VK_NULL_HANDLE)
  {
    return;
  }

  vkDestroyShaderModule(m_device, m_shaderModules[handle.id], nullptr);
  m_shaderModules[handle.id] = VK_NULL_HANDLE;
}

RenderPipelineHandle VulkanPipelineCache::createRenderPipeline(const RenderPipelineDesc& desc)
{
  if (desc.smVert.id >= m_shaderModules.size() || m_shaderModules[desc.smVert.id] == VK_NULL_HANDLE)
  {
    throw std::runtime_error("createRenderPipeline(): invalid vertex shader handle");
  }

  if (desc.smFrag.id >= m_shaderModules.size() || m_shaderModules[desc.smFrag.id] == VK_NULL_HANDLE)
  {
    throw std::runtime_error("createRenderPipeline(): invalid fragment shader handle");
  }

  VkPipelineShaderStageCreateInfo stages[2]{};

  stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  stages[0].module = m_shaderModules[desc.smVert];
  stages[0].pName = "main";

  stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  stages[1].module = m_shaderModules[desc.smFrag];
  stages[1].pName = "main";

  VkSpecializationInfo specInfo{};
  if (!desc.specialization.entries.empty() && desc.specialization.data != nullptr &&
      desc.specialization.dataSize > 0)
  {
    specInfo.mapEntryCount = static_cast<uint32_t>(desc.specialization.entries.size());
    specInfo.pMapEntries = desc.specialization.entries.data();
    specInfo.dataSize = desc.specialization.dataSize;
    specInfo.pData = desc.specialization.data;

    if (desc.specialization.stage == VK_SHADER_STAGE_VERTEX_BIT)
    {
      stages[0].pSpecializationInfo = &specInfo;
    }
    else if (desc.specialization.stage == VK_SHADER_STAGE_FRAGMENT_BIT)
    {
      stages[1].pSpecializationInfo = &specInfo;
    }
  }

  VkPushConstantRange pushRange{};
  VkPushConstantRange* pushRangePtr = nullptr;
  uint32_t pushRangeCount = 0;

  if (desc.pushConstantSize > 0 && desc.pushConstantStages != 0)
  {
    pushRange.stageFlags = desc.pushConstantStages;
    pushRange.offset = 0;
    pushRange.size = desc.pushConstantSize;
    pushRangePtr = &pushRange;
    pushRangeCount = 1;
  }

  VkPipelineLayoutCreateInfo layoutCi{};
  layoutCi.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layoutCi.setLayoutCount = static_cast<uint32_t>(desc.descriptorSetLayouts.size());
  layoutCi.pSetLayouts = desc.descriptorSetLayouts.data();
  layoutCi.pushConstantRangeCount = pushRangeCount;
  layoutCi.pPushConstantRanges = pushRangePtr;

  VkPipelineLayout layout = VK_NULL_HANDLE;
  vkCheck(vkCreatePipelineLayout(m_device, &layoutCi, nullptr, &layout), "vkCreatePipelineLayout");

  VkPipelineVertexInputStateCreateInfo vertexInput{};
  vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInput.vertexBindingDescriptionCount = static_cast<uint32_t>(desc.vertexBindings.size());
  vertexInput.pVertexBindingDescriptions = desc.vertexBindings.data();
  vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(desc.vertexAttributes.size());
  vertexInput.pVertexAttributeDescriptions = desc.vertexAttributes.data();

  VkPipelineInputAssemblyStateCreateInfo ia{};
  ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  ia.topology = desc.topology;
  ia.primitiveRestartEnable = VK_FALSE;

  VkPipelineViewportStateCreateInfo vp{};
  vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  vp.viewportCount = 1;
  vp.scissorCount = 1;

  VkPipelineRasterizationStateCreateInfo rs{};
  rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rs.depthClampEnable = VK_FALSE;
  rs.rasterizerDiscardEnable = VK_FALSE;
  rs.depthBiasEnable = desc.depthBiasEnable ? VK_TRUE : VK_FALSE;
  rs.polygonMode = desc.fillMode;
  rs.cullMode = desc.cullMode;
  rs.frontFace = desc.frontFace;
  rs.lineWidth = 1.0f;

  VkPipelineMultisampleStateCreateInfo msaa{};
  msaa.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  msaa.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineDepthStencilStateCreateInfo ds{};
  ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  ds.depthTestEnable = desc.depthTestEnable ? VK_TRUE : VK_FALSE;
  ds.depthWriteEnable = desc.depthWriteEnable ? VK_TRUE : VK_FALSE;
  ds.depthCompareOp = desc.depthCompareOp;
  ds.depthBoundsTestEnable = VK_FALSE;
  ds.stencilTestEnable = VK_FALSE;

  VkPipelineColorBlendAttachmentState blendAtt{};
  blendAtt.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  blendAtt.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo blend{};
  blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  blend.attachmentCount = 1;
  blend.pAttachments = &blendAtt;

  const VkDynamicState dynamicStates[] = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
      VK_DYNAMIC_STATE_DEPTH_BIAS,
  };

  VkPipelineDynamicStateCreateInfo dyn{};
  dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dyn.dynamicStateCount = static_cast<uint32_t>(std::size(dynamicStates));
  dyn.pDynamicStates = dynamicStates;

  VkPipelineRenderingCreateInfo renderingCi{};
  renderingCi.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  renderingCi.colorAttachmentCount = 1;
  renderingCi.pColorAttachmentFormats = desc.colorFormats.data();
  renderingCi.depthAttachmentFormat = desc.depthFormat;

  VkGraphicsPipelineCreateInfo pipeCi{};
  pipeCi.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeCi.pNext = &renderingCi;
  pipeCi.stageCount = 2;
  pipeCi.pStages = stages;
  pipeCi.pVertexInputState = &vertexInput;
  pipeCi.pInputAssemblyState = &ia;
  pipeCi.pViewportState = &vp;
  pipeCi.pRasterizationState = &rs;
  pipeCi.pMultisampleState = &msaa;
  pipeCi.pColorBlendState = &blend;
  pipeCi.pDynamicState = &dyn;
  pipeCi.layout = layout;
  pipeCi.renderPass = VK_NULL_HANDLE;
  pipeCi.subpass = 0;
  pipeCi.pDepthStencilState = &ds;

  VkPipeline pipeline = VK_NULL_HANDLE;
  VkResult result =
      vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipeCi, nullptr, &pipeline);
  if (result != VK_SUCCESS)
  {
    vkDestroyPipelineLayout(m_device, layout, nullptr);
    throw std::runtime_error("vkCreateGraphicsPipelines failed");
  }

  m_pipelines.push_back({layout, pipeline});
  return RenderPipelineHandle{static_cast<uint32_t>(m_pipelines.size() - 1)};
}

void VulkanPipelineCache::destroyRenderPipeline(RenderPipelineHandle handle)
{
  if (handle.id >= m_pipelines.size())
  {
    return;
  }

  if (m_pipelines[handle.id].pipeline != VK_NULL_HANDLE)
  {
    vkDestroyPipeline(m_device, m_pipelines[handle.id].pipeline, nullptr);
    m_pipelines[handle.id].pipeline = VK_NULL_HANDLE;
  }

  if (m_pipelines[handle.id].layout != VK_NULL_HANDLE)
  {
    vkDestroyPipelineLayout(m_device, m_pipelines[handle.id].layout, nullptr);
    m_pipelines[handle.id].layout = VK_NULL_HANDLE;
  }
}

VkPipeline VulkanPipelineCache::getPipeline(RenderPipelineHandle handle) const
{
  if (handle.id >= m_pipelines.size() || m_pipelines[handle.id].pipeline == VK_NULL_HANDLE)
  {
    throw std::runtime_error("getPipeline(): invalid pipeline handle");
  }

  return m_pipelines[handle.id].pipeline;
}

VkPipelineLayout VulkanPipelineCache::getPipelineLayout(RenderPipelineHandle handle) const
{
  if (handle.id >= m_pipelines.size() || m_pipelines[handle.id].layout == VK_NULL_HANDLE)
  {
    throw std::runtime_error("getPipelineLayout(): invalid pipeline handle");
  }

  return m_pipelines[handle.id].layout;
}

}  // namespace eng