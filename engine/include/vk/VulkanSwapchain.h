#pragma once

#include <vulkan/vulkan.h>

#include "vk/VulkanAllocator.h"

#include <cstdint>
#include <vector>

struct GLFWwindow;

namespace eng
{
class VulkanSwapchain
{
 public:
  VulkanSwapchain() = default;
  ~VulkanSwapchain();

  VulkanSwapchain(const VulkanSwapchain&) = delete;
  VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;

  void create(
      VkPhysicalDevice gpu,
      VkDevice device,
      VulkanAllocator* allocator,
      VkSurfaceKHR surface,
      GLFWwindow* window,
      uint32_t graphicsFamily,
      uint32_t presentFamily,
      VkPresentModeKHR preferredPresentMode,
      VkFormat preferredColorFormat);

  void destroy();
  void recreate(GLFWwindow* window);

  VkSwapchainKHR handle() const { return m_swapchain; }
  VkFormat format() const { return m_format; }
  VkExtent2D extent() const { return m_extent; }

  uint32_t imageCount() const { return static_cast<uint32_t>(m_images.size()); }

  VkFormat depthFormat() const { return m_depthFormat; }
  VkImageView depthView() const { return m_depthImageView; }
  bool hasDepth() const { return m_depthImage != VK_NULL_HANDLE; }

  void transitionDepthForAttachment(VkCommandBuffer cmd);

  VkImage image(uint32_t index) const { return m_images[index]; }
  VkImageView imageView(uint32_t index) const { return m_imageViews[index]; }
  VkSemaphore renderCompleteSemaphore(uint32_t imageIndex) const
  {
    return m_renderCompleteSemaphores[imageIndex];
  }

  void transitionImageForColorAttachment(VkCommandBuffer cmd, uint32_t imageIndex);
  void transitionImageForPresent(VkCommandBuffer cmd, uint32_t imageIndex);

  void createDepthResources();
  void destroyDepthResources();
  static VkFormat chooseDepthFormat(VkPhysicalDevice gpu);

 private:
  void createInternal(GLFWwindow* window);

 private:
  VkPhysicalDevice m_gpu = VK_NULL_HANDLE;
  VkDevice m_device = VK_NULL_HANDLE;
  VkSurfaceKHR m_surface = VK_NULL_HANDLE;

  uint32_t m_graphicsFamily = 0;
  uint32_t m_presentFamily = 0;

  VkPresentModeKHR m_preferredPresentMode = VK_PRESENT_MODE_FIFO_KHR;
  VkFormat m_preferredColorFormat = VK_FORMAT_B8G8R8A8_UNORM;

  VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
  VkFormat m_format = VK_FORMAT_UNDEFINED;
  VkExtent2D m_extent{};

  std::vector<VkImage> m_images;
  std::vector<VkImageView> m_imageViews;
  std::vector<bool> m_imageInitialized;
  std::vector<VkSemaphore> m_renderCompleteSemaphores;

  VulkanAllocator* m_allocator = nullptr;

  VkImage m_depthImage = VK_NULL_HANDLE;
  VmaAllocation m_depthAllocation = nullptr;
  VkImageView m_depthImageView = VK_NULL_HANDLE;
  VkFormat m_depthFormat = VK_FORMAT_UNDEFINED;
  bool m_depthInitialized = false;
};

}  // namespace eng