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
  void cmdDraw(
      uint32_t vertexCount,
      uint32_t instanceCount = 1,
      uint32_t firstVertex = 0,
      uint32_t firstInstance = 0) override;
  void cmdPushDebugGroupLabel(const char* name, uint32_t rgba8) override;
  void cmdPopDebugGroupLabel() override;
  void cmdEndRendering() override;

 private:
  VkDevice m_device = VK_NULL_HANDLE;
  VulkanSwapchain* m_swapchain = nullptr;
  IVulkanPipelineProvider* m_pipelineProvider = nullptr;

  PFN_vkCmdBeginDebugUtilsLabelEXT m_beginLabelFn = nullptr;
  PFN_vkCmdEndDebugUtilsLabelEXT m_endLabelFn = nullptr;

  VkCommandBuffer m_cmd = VK_NULL_HANDLE;
  bool m_isRendering = false;
  TextureHandle m_activeColorImage = kInvalidHandle;
};

}  // namespace eng