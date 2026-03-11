#include "vk/VulkanBuffer.h"

#include <cstring>
#include <stdexcept>
#include <utility>

namespace eng
{
namespace
{
void vkCheck(VkResult result, const char* what)
{
  if (result != VK_SUCCESS)
  {
    throw std::runtime_error(std::string("Vulkan error at: ") + what);
  }
}
}  // namespace

VulkanBuffer::~VulkanBuffer()
{
  destroy();
}

VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept
    : m_allocator(other.m_allocator),
      m_buffer(other.m_buffer),
      m_allocation(other.m_allocation),
      m_allocationInfo(other.m_allocationInfo),
      m_size(other.m_size),
      m_usage(other.m_usage),
      m_mappedData(other.m_mappedData)
{
  other.m_allocator = nullptr;
  other.m_buffer = VK_NULL_HANDLE;
  other.m_allocation = nullptr;
  other.m_allocationInfo = {};
  other.m_size = 0;
  other.m_usage = 0;
  other.m_mappedData = nullptr;
}

VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer&& other) noexcept
{
  if (this != &other)
  {
    destroy();

    m_allocator = other.m_allocator;
    m_buffer = other.m_buffer;
    m_allocation = other.m_allocation;
    m_allocationInfo = other.m_allocationInfo;
    m_size = other.m_size;
    m_usage = other.m_usage;
    m_mappedData = other.m_mappedData;

    other.m_allocator = nullptr;
    other.m_buffer = VK_NULL_HANDLE;
    other.m_allocation = nullptr;
    other.m_allocationInfo = {};
    other.m_size = 0;
    other.m_usage = 0;
    other.m_mappedData = nullptr;
  }

  return *this;
}

void VulkanBuffer::create(
    VulkanAllocator* allocator,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    BufferMemoryUsage memoryUsage,
    VmaAllocationCreateFlags allocationFlags)
{
  if (allocator == nullptr || !allocator->isValid())
  {
    throw std::runtime_error("VulkanBuffer::create(): invalid allocator");
  }

  if (m_buffer != VK_NULL_HANDLE)
  {
    throw std::runtime_error("VulkanBuffer::create(): buffer already created");
  }

  if (size == 0)
  {
    throw std::runtime_error("VulkanBuffer::create(): size must be > 0");
  }

  m_allocator = allocator;
  m_size = size;
  m_usage = usage;

  VkBufferCreateInfo bufferCi{};
  bufferCi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferCi.size = size;
  bufferCi.usage = usage;
  bufferCi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo allocCi{};
  allocCi.usage = toVmaMemoryUsage(memoryUsage);
  allocCi.flags = allocationFlags;

  const VkResult result = vmaCreateBuffer(
      m_allocator->handle(), &bufferCi, &allocCi, &m_buffer, &m_allocation, &m_allocationInfo);

  if (result != VK_SUCCESS)
  {
    m_allocator = nullptr;
    m_size = 0;
    m_usage = 0;
    throw std::runtime_error("VulkanBuffer::create(): vmaCreateBuffer failed");
  }
}

void VulkanBuffer::destroy()
{
  if (m_allocator != nullptr && m_buffer != VK_NULL_HANDLE && m_allocation != nullptr)
  {
    if (m_mappedData != nullptr)
    {
      vmaUnmapMemory(m_allocator->handle(), m_allocation);
      m_mappedData = nullptr;
    }

    vmaDestroyBuffer(m_allocator->handle(), m_buffer, m_allocation);
  }

  m_buffer = VK_NULL_HANDLE;
  m_allocation = nullptr;
  m_allocationInfo = {};
  m_size = 0;
  m_usage = 0;
  m_allocator = nullptr;
  m_mappedData = nullptr;
}

void* VulkanBuffer::map()
{
  if (m_allocator == nullptr || m_allocation == nullptr)
  {
    throw std::runtime_error("VulkanBuffer::map(): buffer is not initialized");
  }

  if (m_mappedData != nullptr)
  {
    return m_mappedData;
  }

  void* mapped = nullptr;
  vkCheck(vmaMapMemory(m_allocator->handle(), m_allocation, &mapped), "vmaMapMemory");

  m_mappedData = mapped;
  return m_mappedData;
}

void VulkanBuffer::unmap()
{
  if (m_allocator == nullptr || m_allocation == nullptr)
  {
    return;
  }

  if (m_mappedData != nullptr)
  {
    vmaUnmapMemory(m_allocator->handle(), m_allocation);
    m_mappedData = nullptr;
  }
}

void VulkanBuffer::upload(const void* data, size_t sizeBytes, size_t offsetBytes)
{
  if (data == nullptr)
  {
    throw std::runtime_error("VulkanBuffer::upload(): data is null");
  }

  if (offsetBytes + sizeBytes > static_cast<size_t>(m_size))
  {
    throw std::runtime_error("VulkanBuffer::upload(): upload range exceeds buffer size");
  }

  std::byte* dst = static_cast<std::byte*>(map());
  std::memcpy(dst + offsetBytes, data, sizeBytes);

  vmaFlushAllocation(
      m_allocator->handle(),
      m_allocation,
      static_cast<VkDeviceSize>(offsetBytes),
      static_cast<VkDeviceSize>(sizeBytes));
}

VmaMemoryUsage VulkanBuffer::toVmaMemoryUsage(BufferMemoryUsage usage)
{
  switch (usage)
  {
    case BufferMemoryUsage::GpuOnly:
      return VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

    case BufferMemoryUsage::CpuToGpu:
      return VMA_MEMORY_USAGE_AUTO_PREFER_HOST;

    case BufferMemoryUsage::GpuToCpu:
      return VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
  }

  return VMA_MEMORY_USAGE_AUTO;
}

}  // namespace eng