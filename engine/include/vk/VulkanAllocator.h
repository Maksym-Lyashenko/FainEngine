#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace eng
{
class VulkanAllocator
{
 public:
  VulkanAllocator() = default;
  ~VulkanAllocator();

  VulkanAllocator(const VulkanAllocator&) = delete;
  VulkanAllocator& operator=(const VulkanAllocator&) = delete;

  void create(
      VkInstance instance,
      VkPhysicalDevice physicalDevice,
      VkDevice device,
      uint32_t vulkanApiVersion = VK_API_VERSION_1_3);

  void destroy();

  VmaAllocator handle() const { return m_allocator; }
  bool isValid() const { return m_allocator != nullptr; }

 private:
  VmaAllocator m_allocator = nullptr;
};

}  // namespace eng