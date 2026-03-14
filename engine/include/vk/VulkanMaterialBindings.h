#pragma once

#include "graphics/Texture2D.h"
#include "graphics/TextureCube.h"
#include "vk/VulkanBuffer.h"
#include "vk/VulkanContext.h"

#include <vulkan/vulkan.h>

namespace eng
{
class VulkanMaterialBindings
{
 public:
  VulkanMaterialBindings() = default;
  ~VulkanMaterialBindings();

  VulkanMaterialBindings(const VulkanMaterialBindings&) = delete;
  VulkanMaterialBindings& operator=(const VulkanMaterialBindings&) = delete;

  VulkanMaterialBindings(VulkanMaterialBindings&& other) noexcept;
  VulkanMaterialBindings& operator=(VulkanMaterialBindings&& other) noexcept;

  void create(
      IContext& ctx, VkBuffer uniformBuffer, VkDeviceSize uniformSize, const Texture2D& texture);

  void create(
      IContext& ctx, VkBuffer uniformBuffer, VkDeviceSize uniformSize, const Texture2D& texture,
      const TextureCube& cubeTexture);

  void destroy();

  bool isValid() const { return m_layout != VK_NULL_HANDLE && m_set != VK_NULL_HANDLE; }

  VkDescriptorSetLayout layout() const { return m_layout; }
  VkDescriptorSet set() const { return m_set; }

  void bind(ICommandBuffer& cmd, uint32_t setIndex = 0) const;

 private:
  VkDevice m_device = VK_NULL_HANDLE;
  VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
  VkDescriptorPool m_pool = VK_NULL_HANDLE;
  VkDescriptorSet m_set = VK_NULL_HANDLE;
};

}  // namespace eng