#pragma once

#include "vk/VulkanAllocator.h"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <cstddef>
#include <cstdint>

namespace eng
{
enum class BufferMemoryUsage : uint8_t
{
  GpuOnly,
  CpuToGpu,
  GpuToCpu
};

class VulkanBuffer
{
 public:
  VulkanBuffer() = default;
  ~VulkanBuffer();

  VulkanBuffer(const VulkanBuffer&) = delete;
  VulkanBuffer& operator=(const VulkanBuffer&) = delete;

  VulkanBuffer(VulkanBuffer&& other) noexcept;
  VulkanBuffer& operator=(VulkanBuffer&& other) noexcept;

  void create(
      VulkanAllocator* allocator,
      VkDeviceSize size,
      VkBufferUsageFlags usage,
      BufferMemoryUsage memoryUsage,
      VmaAllocationCreateFlags allocationFlags = 0);

  void destroy();

  VkBuffer handle() const { return m_buffer; }
  VmaAllocation allocation() const { return m_allocation; }
  const VmaAllocationInfo& allocationInfo() const { return m_allocationInfo; }

  VkDeviceSize size() const { return m_size; }
  VkBufferUsageFlags usage() const { return m_usage; }

  bool isValid() const { return m_buffer != VK_NULL_HANDLE; }
  bool isMapped() const { return m_mappedData != nullptr; }

  void* map();
  void unmap();

  void upload(const void* data, size_t sizeBytes, size_t offsetBytes = 0);

 private:
  static VmaMemoryUsage toVmaMemoryUsage(BufferMemoryUsage usage);

 private:
  VulkanAllocator* m_allocator = nullptr;
  VkBuffer m_buffer = VK_NULL_HANDLE;
  VmaAllocation m_allocation = nullptr;
  VmaAllocationInfo m_allocationInfo{};

  VkDeviceSize m_size = 0;
  VkBufferUsageFlags m_usage = 0;

  void* m_mappedData = nullptr;
};

}  // namespace eng