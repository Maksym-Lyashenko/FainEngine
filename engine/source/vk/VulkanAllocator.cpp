#include "vk/VulkanAllocator.h"

#include <stdexcept>

namespace eng
{
VulkanAllocator::~VulkanAllocator()
{
  destroy();
}

void VulkanAllocator::create(
    VkInstance instance,
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    uint32_t vulkanApiVersion)
{
  if (m_allocator != nullptr)
  {
    throw std::runtime_error("VulkanAllocator::create(): allocator already created");
  }

  VmaAllocatorCreateInfo ci{};
  ci.instance = instance;
  ci.physicalDevice = physicalDevice;
  ci.device = device;
  ci.vulkanApiVersion = vulkanApiVersion;

  const VkResult result = vmaCreateAllocator(&ci, &m_allocator);
  if (result != VK_SUCCESS)
  {
    throw std::runtime_error("VulkanAllocator::create(): vmaCreateAllocator failed");
  }
}

void VulkanAllocator::destroy()
{
  if (m_allocator != nullptr)
  {
    vmaDestroyAllocator(m_allocator);
    m_allocator = nullptr;
  }
}

}  // namespace eng