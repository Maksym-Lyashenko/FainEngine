#pragma once

#include "graphics/TextureCube.h"
#include "vk/VulkanCommon.h"

namespace eng
{
class VulkanSkyboxBindings
{
 public:
  VulkanSkyboxBindings() = default;
  ~VulkanSkyboxBindings();

  VulkanSkyboxBindings(const VulkanSkyboxBindings&) = delete;
  VulkanSkyboxBindings& operator=(const VulkanSkyboxBindings&) = delete;

  VulkanSkyboxBindings(VulkanSkyboxBindings&& other) noexcept;
  VulkanSkyboxBindings& operator=(VulkanSkyboxBindings&& other) noexcept;

  void create(
      IContext& ctx,
      VkBuffer uniformBuffer,
      VkDeviceSize uniformSize,
      const TextureCube& cubeTexture);

  void destroy();

  bool isValid() const { return m_layout != VK_NULL_HANDLE && m_set != VK_NULL_HANDLE; }

  VkDescriptorSetLayout layout() const { return m_layout; }

  void bind(ICommandBuffer& cmd, uint32_t setIndex = 0) const;

 private:
  VkDevice m_device = VK_NULL_HANDLE;
  VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
  VkDescriptorPool m_pool = VK_NULL_HANDLE;
  VkDescriptorSet m_set = VK_NULL_HANDLE;
};

}  // namespace eng