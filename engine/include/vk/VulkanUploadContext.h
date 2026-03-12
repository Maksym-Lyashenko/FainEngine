#pragma once

#include "vk/VulkanAllocator.h"
#include "vk/VulkanBuffer.h"

#include <functional>
#include <vulkan/vulkan.h>

namespace eng
{
class VulkanUploadContext
{
 public:
  VulkanUploadContext() = default;
  ~VulkanUploadContext();

  VulkanUploadContext(const VulkanUploadContext&) = delete;
  VulkanUploadContext& operator=(const VulkanUploadContext&) = delete;

  void create(VkDevice device, uint32_t graphicsFamily, VkQueue graphicsQueue);
  void destroy();

  void immediateSubmit(const std::function<void(VkCommandBuffer cmd)>& recorder);

  void uploadBuffer(
      VulkanAllocator* allocator,
      const void* data,
      size_t sizeBytes,
      VulkanBuffer& dstBuffer,
      VkDeviceSize dstOffset = 0);

  void uploadImage2D(
      VulkanAllocator* allocator,
      const void* data,
      size_t sizeBytes,
      VkImage dstImage,
      uint32_t width,
      uint32_t height);

 private:
  VkDevice m_device = VK_NULL_HANDLE;
  VkQueue m_graphicsQueue = VK_NULL_HANDLE;

  VkCommandPool m_commandPool = VK_NULL_HANDLE;
  VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
  VkFence m_fence = VK_NULL_HANDLE;
};

}  // namespace eng