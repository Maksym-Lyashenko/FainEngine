#include "vk/VulkanUploadContext.h"

#include <stdexcept>

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

VulkanUploadContext::~VulkanUploadContext()
{
  destroy();
}

void VulkanUploadContext::create(VkDevice device, uint32_t graphicsFamily, VkQueue graphicsQueue)
{
  if (m_device != VK_NULL_HANDLE)
  {
    throw std::runtime_error("VulkanUploadContext::create(): already created");
  }

  m_device = device;
  m_graphicsQueue = graphicsQueue;

  VkCommandPoolCreateInfo poolCi{};
  poolCi.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolCi.queueFamilyIndex = graphicsFamily;
  poolCi.flags =
      VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  vkCheck(vkCreateCommandPool(m_device, &poolCi, nullptr, &m_commandPool), "vkCreateCommandPool");

  VkCommandBufferAllocateInfo alloc{};
  alloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc.commandPool = m_commandPool;
  alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc.commandBufferCount = 1;
  vkCheck(vkAllocateCommandBuffers(m_device, &alloc, &m_commandBuffer), "vkAllocateCommandBuffers");

  VkFenceCreateInfo fenceCi{};
  fenceCi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceCi.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  vkCheck(vkCreateFence(m_device, &fenceCi, nullptr, &m_fence), "vkCreateFence");
}

void VulkanUploadContext::destroy()
{
  if (m_device != VK_NULL_HANDLE)
  {
    if (m_fence != VK_NULL_HANDLE)
    {
      vkDestroyFence(m_device, m_fence, nullptr);
      m_fence = VK_NULL_HANDLE;
    }

    if (m_commandPool != VK_NULL_HANDLE)
    {
      vkDestroyCommandPool(m_device, m_commandPool, nullptr);
      m_commandPool = VK_NULL_HANDLE;
    }
  }

  m_commandBuffer = VK_NULL_HANDLE;
  m_graphicsQueue = VK_NULL_HANDLE;
  m_device = VK_NULL_HANDLE;
}

void VulkanUploadContext::immediateSubmit(const std::function<void(VkCommandBuffer cmd)>& recorder)
{
  if (m_device == VK_NULL_HANDLE || m_commandPool == VK_NULL_HANDLE ||
      m_commandBuffer == VK_NULL_HANDLE)
  {
    throw std::runtime_error("VulkanUploadContext::immediateSubmit(): context is not initialized");
  }

  vkCheck(vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, UINT64_MAX), "vkWaitForFences");
  vkCheck(vkResetFences(m_device, 1, &m_fence), "vkResetFences");
  vkCheck(vkResetCommandPool(m_device, m_commandPool, 0), "vkResetCommandPool");

  VkCommandBufferBeginInfo begin{};
  begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkCheck(vkBeginCommandBuffer(m_commandBuffer, &begin), "vkBeginCommandBuffer");

  recorder(m_commandBuffer);

  vkCheck(vkEndCommandBuffer(m_commandBuffer), "vkEndCommandBuffer");

  VkSubmitInfo submit{};
  submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit.commandBufferCount = 1;
  submit.pCommandBuffers = &m_commandBuffer;

  vkCheck(vkQueueSubmit(m_graphicsQueue, 1, &submit, m_fence), "vkQueueSubmit");
  vkCheck(vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, UINT64_MAX), "vkWaitForFences");
}

void VulkanUploadContext::uploadBuffer(
    VulkanAllocator* allocator,
    const void* data,
    size_t sizeBytes,
    VulkanBuffer& dstBuffer,
    VkDeviceSize dstOffset)
{
  if (allocator == nullptr || !allocator->isValid())
  {
    throw std::runtime_error("VulkanUploadContext::uploadBuffer(): invalid allocator");
  }

  if (data == nullptr)
  {
    throw std::runtime_error("VulkanUploadContext::uploadBuffer(): data is null");
  }

  if (!dstBuffer.isValid())
  {
    throw std::runtime_error("VulkanUploadContext::uploadBuffer(): destination buffer is invalid");
  }

  if (dstOffset + static_cast<VkDeviceSize>(sizeBytes) > dstBuffer.size())
  {
    throw std::runtime_error(
        "VulkanUploadContext::uploadBuffer(): upload range exceeds destination buffer size");
  }

  VulkanBuffer staging;
  staging.create(
      allocator,
      static_cast<VkDeviceSize>(sizeBytes),
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      BufferMemoryUsage::CpuToGpu,
      VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

  staging.upload(data, sizeBytes);

  immediateSubmit(
      [&](VkCommandBuffer cmd)
      {
        VkBufferCopy region{};
        region.srcOffset = 0;
        region.dstOffset = dstOffset;
        region.size = static_cast<VkDeviceSize>(sizeBytes);

        vkCmdCopyBuffer(cmd, staging.handle(), dstBuffer.handle(), 1, &region);
      });
}

}  // namespace eng