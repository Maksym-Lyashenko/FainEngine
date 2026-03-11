#pragma once

#include "vk/VulkanCommon.h"
#include "vk/VulkanSwapchain.h"

namespace eng
{
class IVulkanPipelineProvider
{
 public:
  virtual ~IVulkanPipelineProvider() = default;
  virtual VkPipeline getPipeline(RenderPipelineHandle handle) const = 0;
  virtual VkPipelineLayout getPipelineLayout(RenderPipelineHandle handle) const = 0;
};

class VulkanCommandBuffer final : public ICommandBuffer
{
 public:
  VulkanCommandBuffer() = default;

  void setup(
      VkDevice device,
      VulkanSwapchain* swapchain,
      IVulkanPipelineProvider* pipelineProvider,
      PFN_vkCmdBeginDebugUtilsLabelEXT beginLabelFn,
      PFN_vkCmdEndDebugUtilsLabelEXT endLabelFn);

  void begin(VkCommandBuffer cmd);
  void finalizeIfNeeded();

  void cmdBeginRendering(const BeginRenderingDesc& desc, const RenderingTargets& targets) override;
  void cmdBindRenderPipeline(RenderPipelineHandle pipeline) override;
  void cmdBindVertexBuffer(VkBuffer buffer, VkDeviceSize offset = 0) override;
  void cmdBindIndexBuffer(VkBuffer buffer, VkDeviceSize offset = 0) override;

  void cmdPushConstants(const void* data, uint32_t size, uint32_t offset = 0) override;

  void cmdDraw(
      uint32_t vertexCount,
      uint32_t instanceCount = 1,
      uint32_t firstVertex = 0,
      uint32_t firstInstance = 0) override;
  void cmdDrawIndexed(
      uint32_t indexCount,
      uint32_t instanceCount = 1,
      uint32_t firstIndex = 0,
      int32_t vertexOffset = 0,
      uint32_t firstInstance = 0) override;
  void cmdPushDebugGroupLabel(const char* name, uint32_t rgba8) override;
  void cmdPopDebugGroupLabel() override;
  void cmdEndRendering() override;

 private:
  VkDevice m_device = VK_NULL_HANDLE;
  VulkanSwapchain* m_swapchain = nullptr;
  IVulkanPipelineProvider* m_pipelineProvider = nullptr;

  VkPipelineLayout m_boundPipelineLayout = VK_NULL_HANDLE;

  PFN_vkCmdBeginDebugUtilsLabelEXT m_beginLabelFn = nullptr;
  PFN_vkCmdEndDebugUtilsLabelEXT m_endLabelFn = nullptr;

  VkCommandBuffer m_cmd = VK_NULL_HANDLE;
  bool m_isRendering = false;
  TextureHandle m_activeColorImage = kInvalidHandle;
};

}  // namespace eng