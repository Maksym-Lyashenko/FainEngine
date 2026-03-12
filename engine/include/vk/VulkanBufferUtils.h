#pragma once

#include "vk/VulkanBuffer.h"
#include "vk/VulkanContext.h"

#include <span>
#include <type_traits>
#include <cstdint>

namespace eng
{
inline VulkanBuffer CreateDeviceBuffer(
    IContext& ctx, const void* data, size_t sizeBytes, VkBufferUsageFlags usage)
{
  VulkanBuffer buffer;
  buffer.create(
      &ctx.allocator(),
      static_cast<VkDeviceSize>(sizeBytes),
      usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      BufferMemoryUsage::GpuOnly);

  if (data != nullptr && sizeBytes > 0)
  {
    ctx.uploadContext().uploadBuffer(&ctx.allocator(), data, sizeBytes, buffer);
  }

  return buffer;
}

template <typename T>
inline VulkanBuffer CreateVertexBuffer(IContext& ctx, std::span<const T> data)
{
  static_assert(std::is_trivially_copyable_v<T>);

  return CreateDeviceBuffer(ctx, data.data(), data.size_bytes(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
}

template <typename T>
inline VulkanBuffer CreateIndexBuffer(IContext& ctx, std::span<const T> data)
{
  static_assert(std::is_trivially_copyable_v<T>);
  static_assert(std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t>);

  return CreateDeviceBuffer(ctx, data.data(), data.size_bytes(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
}

template <typename T>
inline VulkanBuffer CreateStorageBuffer(IContext& ctx, std::span<const T> data)
{
  static_assert(std::is_trivially_copyable_v<T>);

  return CreateDeviceBuffer(
      ctx, data.data(), data.size_bytes(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
}

}  // namespace eng