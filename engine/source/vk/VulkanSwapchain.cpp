#include "vk/VulkanSwapchain.h"

#include <GLFW/glfw3.h>

#include <vk_mem_alloc.h>

#include <algorithm>
#include <stdexcept>
#include <vector>
#include <array>

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

struct SwapchainSupport
{
  VkSurfaceCapabilitiesKHR caps{};
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

SwapchainSupport querySwapchainSupport(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
  SwapchainSupport sc{};

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &sc.caps);

  uint32_t formatCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, nullptr);
  sc.formats.resize(formatCount);
  if (formatCount > 0)
  {
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, sc.formats.data());
  }

  uint32_t presentModeCount = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, nullptr);
  sc.presentModes.resize(presentModeCount);
  if (presentModeCount > 0)
  {
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        gpu, surface, &presentModeCount, sc.presentModes.data());
  }

  return sc;
}

VkSurfaceFormatKHR chooseSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& formats, VkFormat preferred)
{
  for (const auto& f : formats)
  {
    if (f.format == preferred && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
    {
      return f;
    }
  }

  for (const auto& f : formats)
  {
    if (f.format == preferred)
    {
      return f;
    }
  }

  return formats.front();
}

VkPresentModeKHR choosePresentMode(
    const std::vector<VkPresentModeKHR>& presentModes, VkPresentModeKHR preferred)
{
  for (VkPresentModeKHR mode : presentModes)
  {
    if (mode == preferred)
    {
      return mode;
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& caps)
{
  if (caps.currentExtent.width != UINT32_MAX)
  {
    return caps.currentExtent;
  }

  int width = 0;
  int height = 0;
  glfwGetFramebufferSize(window, &width, &height);

  VkExtent2D extent{};
  extent.width = std::clamp<uint32_t>(
      static_cast<uint32_t>(width), caps.minImageExtent.width, caps.maxImageExtent.width);
  extent.height = std::clamp<uint32_t>(
      static_cast<uint32_t>(height), caps.minImageExtent.height, caps.maxImageExtent.height);

  return extent;
}

}  // namespace

VulkanSwapchain::~VulkanSwapchain()
{
  destroy();
}

void VulkanSwapchain::create(
    VkPhysicalDevice gpu,
    VkDevice device,
    VulkanAllocator* allocator,
    VkSurfaceKHR surface,
    GLFWwindow* window,
    uint32_t graphicsFamily,
    uint32_t presentFamily,
    VkPresentModeKHR preferredPresentMode,
    VkFormat preferredColorFormat)
{
  m_gpu = gpu;
  m_device = device;
  m_allocator = allocator;
  m_surface = surface;
  m_graphicsFamily = graphicsFamily;
  m_presentFamily = presentFamily;
  m_preferredPresentMode = preferredPresentMode;
  m_preferredColorFormat = preferredColorFormat;

  createInternal(window);
}

void VulkanSwapchain::destroy()
{
  destroyDepthResources();
  for (VkSemaphore sem : m_renderCompleteSemaphores)
  {
    if (sem != VK_NULL_HANDLE)
    {
      vkDestroySemaphore(m_device, sem, nullptr);
    }
  }
  m_renderCompleteSemaphores.clear();

  for (VkImageView view : m_imageViews)
  {
    if (view != VK_NULL_HANDLE)
    {
      vkDestroyImageView(m_device, view, nullptr);
    }
  }
  m_imageViews.clear();

  m_images.clear();
  m_imageInitialized.clear();

  if (m_swapchain != VK_NULL_HANDLE)
  {
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    m_swapchain = VK_NULL_HANDLE;
  }

  m_format = VK_FORMAT_UNDEFINED;
  m_extent = {};
}

void VulkanSwapchain::recreate(GLFWwindow* window)
{
  int width = 0;
  int height = 0;

  do
  {
    glfwGetFramebufferSize(window, &width, &height);
    glfwWaitEvents();
  } while (width == 0 || height == 0);

  vkDeviceWaitIdle(m_device);

  destroy();
  createInternal(window);
}

void VulkanSwapchain::createInternal(GLFWwindow* window)
{
  const SwapchainSupport sc = querySwapchainSupport(m_gpu, m_surface);

  if (sc.formats.empty())
  {
    throw std::runtime_error("No surface formats available for swapchain");
  }

  if (sc.presentModes.empty())
  {
    throw std::runtime_error("No present modes available for swapchain");
  }

  const VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(sc.formats, m_preferredColorFormat);
  const VkPresentModeKHR presentMode = choosePresentMode(sc.presentModes, m_preferredPresentMode);
  const VkExtent2D swapExtent = chooseExtent(window, sc.caps);

  uint32_t imageCount = sc.caps.minImageCount + 1;
  if (sc.caps.maxImageCount > 0 && imageCount > sc.caps.maxImageCount)
  {
    imageCount = sc.caps.maxImageCount;
  }

  VkSwapchainCreateInfoKHR ci{};
  ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  ci.surface = m_surface;
  ci.minImageCount = imageCount;
  ci.imageFormat = surfaceFormat.format;
  ci.imageColorSpace = surfaceFormat.colorSpace;
  ci.imageExtent = swapExtent;
  ci.imageArrayLayers = 1;
  ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  const uint32_t queueFamilyIndices[] = {m_graphicsFamily, m_presentFamily};
  if (m_graphicsFamily != m_presentFamily)
  {
    ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    ci.queueFamilyIndexCount = 2;
    ci.pQueueFamilyIndices = queueFamilyIndices;
  }
  else
  {
    ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  ci.preTransform = sc.caps.currentTransform;
  ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  ci.presentMode = presentMode;
  ci.clipped = VK_TRUE;
  ci.oldSwapchain = VK_NULL_HANDLE;

  vkCheck(vkCreateSwapchainKHR(m_device, &ci, nullptr, &m_swapchain), "vkCreateSwapchainKHR");

  m_format = surfaceFormat.format;
  m_extent = swapExtent;

  uint32_t actualImageCount = 0;
  vkGetSwapchainImagesKHR(m_device, m_swapchain, &actualImageCount, nullptr);

  m_images.resize(actualImageCount);
  vkGetSwapchainImagesKHR(m_device, m_swapchain, &actualImageCount, m_images.data());

  m_imageViews.resize(actualImageCount, VK_NULL_HANDLE);
  m_imageInitialized.assign(actualImageCount, false);
  m_renderCompleteSemaphores.resize(actualImageCount, VK_NULL_HANDLE);

  for (uint32_t i = 0; i < actualImageCount; ++i)
  {
    VkImageViewCreateInfo viewCi{};
    viewCi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCi.image = m_images[i];
    viewCi.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCi.format = m_format;
    viewCi.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewCi.subresourceRange.baseMipLevel = 0;
    viewCi.subresourceRange.levelCount = 1;
    viewCi.subresourceRange.baseArrayLayer = 0;
    viewCi.subresourceRange.layerCount = 1;

    vkCheck(vkCreateImageView(m_device, &viewCi, nullptr, &m_imageViews[i]), "vkCreateImageView");
  }

  createDepthResources();

  VkSemaphoreCreateInfo semCi{};
  semCi.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  for (uint32_t i = 0; i < actualImageCount; ++i)
  {
    vkCheck(
        vkCreateSemaphore(m_device, &semCi, nullptr, &m_renderCompleteSemaphores[i]),
        "vkCreateSemaphore renderComplete");
  }
}

void VulkanSwapchain::transitionImageForColorAttachment(VkCommandBuffer cmd, uint32_t imageIndex)
{
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout =
      m_imageInitialized[imageIndex] ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_UNDEFINED;
  barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = m_images[imageIndex];
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.srcAccessMask = 0;
  barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  const VkPipelineStageFlags srcStage = m_imageInitialized[imageIndex]
                                            ? VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                                            : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

  vkCmdPipelineBarrier(
      cmd, srcStage, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1,
      &barrier);
}

void VulkanSwapchain::transitionImageForPresent(VkCommandBuffer cmd, uint32_t imageIndex)
{
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = m_images[imageIndex];
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  barrier.dstAccessMask = 0;

  vkCmdPipelineBarrier(
      cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0,
      0, nullptr, 0, nullptr, 1, &barrier);

  m_imageInitialized[imageIndex] = true;
}

void VulkanSwapchain::transitionDepthForAttachment(VkCommandBuffer cmd)
{
  if (m_depthImage == VK_NULL_HANDLE)
  {
    return;
  }

  if (m_depthInitialized)
  {
    return;
  }

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = m_depthImage;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.srcAccessMask = 0;
  barrier.dstAccessMask =
      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  vkCmdPipelineBarrier(
      cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0,
      nullptr, 0, nullptr, 1, &barrier);

  m_depthInitialized = true;
}

void VulkanSwapchain::createDepthResources()
{
  if (m_allocator == nullptr || !m_allocator->isValid())
  {
    throw std::runtime_error("VulkanSwapchain::createDepthResources(): invalid allocator");
  }

  m_depthFormat = chooseDepthFormat(m_gpu);

  VkImageCreateInfo imageCi{};
  imageCi.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageCi.imageType = VK_IMAGE_TYPE_2D;
  imageCi.format = m_depthFormat;
  imageCi.extent.width = m_extent.width;
  imageCi.extent.height = m_extent.height;
  imageCi.extent.depth = 1;
  imageCi.mipLevels = 1;
  imageCi.arrayLayers = 1;
  imageCi.samples = VK_SAMPLE_COUNT_1_BIT;
  imageCi.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageCi.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  imageCi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageCi.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  VmaAllocationCreateInfo allocCi{};
  allocCi.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

  vkCheck(
      vmaCreateImage(
          m_allocator->handle(), &imageCi, &allocCi, &m_depthImage, &m_depthAllocation, nullptr),
      "vmaCreateImage depth");

  VkImageViewCreateInfo viewCi{};
  viewCi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewCi.image = m_depthImage;
  viewCi.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewCi.format = m_depthFormat;
  viewCi.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  viewCi.subresourceRange.baseMipLevel = 0;
  viewCi.subresourceRange.levelCount = 1;
  viewCi.subresourceRange.baseArrayLayer = 0;
  viewCi.subresourceRange.layerCount = 1;

  vkCheck(
      vkCreateImageView(m_device, &viewCi, nullptr, &m_depthImageView), "vkCreateImageView depth");

  m_depthInitialized = false;
}

void VulkanSwapchain::destroyDepthResources()
{
  if (m_depthImageView != VK_NULL_HANDLE)
  {
    vkDestroyImageView(m_device, m_depthImageView, nullptr);
    m_depthImageView = VK_NULL_HANDLE;
  }

  if (m_depthImage != VK_NULL_HANDLE && m_depthAllocation != nullptr)
  {
    vmaDestroyImage(m_allocator->handle(), m_depthImage, m_depthAllocation);
    m_depthImage = VK_NULL_HANDLE;
    m_depthAllocation = nullptr;
  }

  m_depthFormat = VK_FORMAT_UNDEFINED;
  m_depthInitialized = false;
}

VkFormat VulkanSwapchain::chooseDepthFormat(VkPhysicalDevice gpu)
{
  const std::array<VkFormat, 3> candidates = {
      VK_FORMAT_D32_SFLOAT,
      VK_FORMAT_D32_SFLOAT_S8_UINT,
      VK_FORMAT_D24_UNORM_S8_UINT,
  };

  for (VkFormat format : candidates)
  {
    VkFormatProperties props{};
    vkGetPhysicalDeviceFormatProperties(gpu, format, &props);

    if ((props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0)
    {
      return format;
    }
  }

  throw std::runtime_error("No suitable depth format found");
}

}  // namespace eng