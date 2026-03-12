#include "vk/VulkanCommandBuffer.h"

#include <array>
#include <stdexcept>

namespace eng
{
namespace
{
std::array<float, 4> unpackRGBA8(uint32_t rgba)
{
  return {
      ((rgba >> 24) & 0xFF) / 255.0f,
      ((rgba >> 16) & 0xFF) / 255.0f,
      ((rgba >> 8) & 0xFF) / 255.0f,
      ((rgba >> 0) & 0xFF) / 255.0f,
  };
}
}  // namespace

void VulkanCommandBuffer::setup(
    VkDevice device,
    VulkanSwapchain* swapchain,
    IVulkanPipelineProvider* pipelineProvider,
    PFN_vkCmdBeginDebugUtilsLabelEXT beginLabelFn,
    PFN_vkCmdEndDebugUtilsLabelEXT endLabelFn)
{
  m_device = device;
  m_swapchain = swapchain;
  m_pipelineProvider = pipelineProvider;
  m_beginLabelFn = beginLabelFn;
  m_endLabelFn = endLabelFn;
}

void VulkanCommandBuffer::begin(VkCommandBuffer cmd)
{
  m_cmd = cmd;
  m_isRendering = false;
  m_activeColorImage = TextureHandle{};
  m_boundPipelineLayout = VK_NULL_HANDLE;
}

void VulkanCommandBuffer::finalizeIfNeeded()
{
  if (m_isRendering)
  {
    cmdEndRendering();
  }
}

void VulkanCommandBuffer::cmdBeginRendering(
    const BeginRenderingDesc& desc, const RenderingTargets& targets)
{
  if (!targets.color[0].valid())
  {
    throw std::runtime_error("cmdBeginRendering(): invalid color target");
  }

  const uint32_t imageIndex = targets.color[0].id;
  m_activeColorImage = targets.color[0];

  m_swapchain->transitionImageForColorAttachment(m_cmd, imageIndex);

  VkAttachmentLoadOp vkLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
  switch (desc.color[0].loadOp)
  {
    case LoadOp::Load:
      vkLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      break;
    case LoadOp::Clear:
      vkLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      break;
    case LoadOp::DontCare:
      vkLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      break;
  }

  VkClearValue clear{};
  clear.color.float32[0] = desc.color[0].clearColor[0];
  clear.color.float32[1] = desc.color[0].clearColor[1];
  clear.color.float32[2] = desc.color[0].clearColor[2];
  clear.color.float32[3] = desc.color[0].clearColor[3];

  VkRenderingAttachmentInfo colorAtt{};
  colorAtt.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  colorAtt.imageView = m_swapchain->imageView(imageIndex);
  colorAtt.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  colorAtt.loadOp = vkLoadOp;
  colorAtt.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAtt.clearValue = clear;

  VkRenderingAttachmentInfo depthAtt{};
  if (desc.useDepth)
  {
    m_swapchain->transitionDepthForAttachment(m_cmd);

    VkAttachmentLoadOp depthLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    switch (desc.depth.loadOp)
    {
      case LoadOp::Load:
        depthLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        break;
      case LoadOp::Clear:
        depthLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        break;
      case LoadOp::DontCare:
        depthLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        break;
    }

    VkClearValue depthClear{};
    depthClear.depthStencil.depth = desc.depth.clearDepth;
    depthClear.depthStencil.stencil = 0;

    depthAtt.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAtt.imageView = m_swapchain->depthView();
    depthAtt.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthAtt.loadOp = depthLoadOp;
    depthAtt.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAtt.clearValue = depthClear;
  }

  VkRenderingInfo ri{};
  ri.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  ri.renderArea.offset = {0, 0};
  ri.renderArea.extent = m_swapchain->extent();
  ri.layerCount = 1;
  ri.colorAttachmentCount = 1;
  ri.pColorAttachments = &colorAtt;
  if (desc.useDepth)
  {
    ri.pDepthAttachment = &depthAtt;
  }

  vkCmdBeginRendering(m_cmd, &ri);
  m_isRendering = true;
}

void VulkanCommandBuffer::cmdBindRenderPipeline(RenderPipelineHandle pipeline)
{
  const VkPipeline vkPipeline = m_pipelineProvider->getPipeline(pipeline);
  if (vkPipeline == VK_NULL_HANDLE)
  {
    throw std::runtime_error("cmdBindRenderPipeline(): invalid pipeline handle");
  }

  vkCmdBindPipeline(m_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);

  m_boundPipelineLayout = m_pipelineProvider->getPipelineLayout(pipeline);

  const VkExtent2D extent = m_swapchain->extent();

  VkViewport vp{};
  vp.x = 0.0f;
  vp.y = static_cast<float>(extent.height);
  vp.width = static_cast<float>(extent.width);
  vp.height = -static_cast<float>(extent.height);
  vp.minDepth = 0.0f;
  vp.maxDepth = 1.0f;

  VkRect2D sc{};
  sc.offset = {0, 0};
  sc.extent = extent;

  vkCmdSetViewport(m_cmd, 0, 1, &vp);
  vkCmdSetScissor(m_cmd, 0, 1, &sc);
}

void VulkanCommandBuffer::cmdBindVertexBuffer(VkBuffer buffer, VkDeviceSize offset)
{
  if (buffer == VK_NULL_HANDLE)
  {
    throw std::runtime_error("cmdBindVertexBuffer(): invalid buffer");
  }

  vkCmdBindVertexBuffers(m_cmd, 0, 1, &buffer, &offset);
}

void VulkanCommandBuffer::cmdBindIndexBuffer(
    VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
{
  if (buffer == VK_NULL_HANDLE)
  {
    throw std::runtime_error("cmdBindIndexBuffer(): invalid buffer");
  }

  vkCmdBindIndexBuffer(m_cmd, buffer, offset, indexType);
}

void VulkanCommandBuffer::cmdBindDescriptorSet(VkDescriptorSet set, uint32_t setIndex)
{
  if (set == VK_NULL_HANDLE)
  {
    throw std::runtime_error("cmdBindDescriptorSet(): invalid descriptor set");
  }

  if (m_boundPipelineLayout == VK_NULL_HANDLE)
  {
    throw std::runtime_error("cmdBindDescriptorSet(): no pipeline layout is currently bound");
  }

  vkCmdBindDescriptorSets(
      m_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_boundPipelineLayout, setIndex, 1, &set, 0, nullptr);
}

void VulkanCommandBuffer::cmdSetDepthBias(float constantFactor, float slopeFactor, float clamp)
{
  vkCmdSetDepthBias(m_cmd, constantFactor, clamp, slopeFactor);
}

void VulkanCommandBuffer::cmdPushConstants(const void* data, uint32_t size, uint32_t offset)
{
  if (data == nullptr || size == 0)
  {
    throw std::runtime_error("cmdPushConstants(): invalid data");
  }

  if (m_boundPipelineLayout == VK_NULL_HANDLE)
  {
    throw std::runtime_error("cmdPushConstants(): no pipeline layout is currently bound");
  }

  vkCmdPushConstants(m_cmd, m_boundPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, offset, size, data);
}

void VulkanCommandBuffer::cmdDraw(
    uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
  vkCmdDraw(m_cmd, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanCommandBuffer::cmdDrawIndexed(
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t vertexOffset,
    uint32_t firstInstance)
{
  vkCmdDrawIndexed(m_cmd, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanCommandBuffer::cmdPushDebugGroupLabel(const char* name, uint32_t rgba8)
{
  if (!m_beginLabelFn)
  {
    return;
  }

  const auto color = unpackRGBA8(rgba8);

  VkDebugUtilsLabelEXT label{};
  label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
  label.pLabelName = name;
  label.color[0] = color[0];
  label.color[1] = color[1];
  label.color[2] = color[2];
  label.color[3] = color[3];

  m_beginLabelFn(m_cmd, &label);
}

void VulkanCommandBuffer::cmdPopDebugGroupLabel()
{
  if (m_endLabelFn)
  {
    m_endLabelFn(m_cmd);
  }
}

void VulkanCommandBuffer::cmdEndRendering()
{
  if (!m_isRendering)
  {
    return;
  }

  vkCmdEndRendering(m_cmd);
  m_swapchain->transitionImageForPresent(m_cmd, m_activeColorImage.id);

  m_isRendering = false;
  m_activeColorImage = TextureHandle{};
}

}  // namespace eng