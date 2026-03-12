#pragma once

#include "vk/VulkanContext.h"

#include <vulkan/vulkan.h>

namespace eng
{
class VulkanCombinedImageSamplerBindings
{
 public:
  VulkanCombinedImageSamplerBindings() = default;
  ~VulkanCombinedImageSamplerBindings();

  VulkanCombinedImageSamplerBindings(const VulkanCombinedImageSamplerBindings&) = delete;
  VulkanCombinedImageSamplerBindings& operator=(const VulkanCombinedImageSamplerBindings&) = delete;

  VulkanCombinedImageSamplerBindings(VulkanCombinedImageSamplerBindings&& other) noexcept;
  VulkanCombinedImageSamplerBindings& operator=(
      VulkanCombinedImageSamplerBindings&& other) noexcept;

  void create(IContext& ctx, VkImageView imageView, VkSampler sampler);
  void destroy();

  bool isValid() const { return m_layout != VK_NULL_HANDLE && m_set != VK_NULL_HANDLE; }

  VkDescriptorSetLayout layout() const { return m_layout; }
  VkDescriptorSet set() const { return m_set; }

 private:
  VkDevice m_device = VK_NULL_HANDLE;
  VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
  VkDescriptorPool m_pool = VK_NULL_HANDLE;
  VkDescriptorSet m_set = VK_NULL_HANDLE;
};

}  // namespace eng