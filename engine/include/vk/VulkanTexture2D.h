#pragma once

#include "vk/VulkanAllocator.h"
#include "vk/VulkanContext.h"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <cstdint>

namespace eng
{
class VulkanTexture2D
{
 public:
  VulkanTexture2D() = default;
  ~VulkanTexture2D();

  VulkanTexture2D(const VulkanTexture2D&) = delete;
  VulkanTexture2D& operator=(const VulkanTexture2D&) = delete;

  VulkanTexture2D(VulkanTexture2D&& other) noexcept;
  VulkanTexture2D& operator=(VulkanTexture2D&& other) noexcept;

  void createRGBA8(IContext& ctx, uint32_t width, uint32_t height, const void* pixels);

  void destroy();

  bool isValid() const { return m_image != VK_NULL_HANDLE; }

  VkImage image() const { return m_image; }
  VkImageView imageView() const { return m_imageView; }
  VkSampler sampler() const { return m_sampler; }
  VkFormat format() const { return m_format; }
  uint32_t width() const { return m_width; }
  uint32_t height() const { return m_height; }

 private:
  VulkanAllocator* m_allocator = nullptr;
  VkDevice m_device = VK_NULL_HANDLE;

  VkImage m_image = VK_NULL_HANDLE;
  VmaAllocation m_allocation = nullptr;
  VkImageView m_imageView = VK_NULL_HANDLE;
  VkSampler m_sampler = VK_NULL_HANDLE;

  VkFormat m_format = VK_FORMAT_UNDEFINED;
  uint32_t m_width = 0;
  uint32_t m_height = 0;
};

}  // namespace eng