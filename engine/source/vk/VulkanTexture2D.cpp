#include "vk/VulkanTexture2D.h"

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

VulkanTexture2D::~VulkanTexture2D()
{
  destroy();
}

VulkanTexture2D::VulkanTexture2D(VulkanTexture2D&& other) noexcept
    : m_allocator(other.m_allocator),
      m_device(other.m_device),
      m_image(other.m_image),
      m_allocation(other.m_allocation),
      m_imageView(other.m_imageView),
      m_sampler(other.m_sampler),
      m_format(other.m_format),
      m_width(other.m_width),
      m_height(other.m_height)
{
  other.m_allocator = nullptr;
  other.m_device = VK_NULL_HANDLE;
  other.m_image = VK_NULL_HANDLE;
  other.m_allocation = nullptr;
  other.m_imageView = VK_NULL_HANDLE;
  other.m_sampler = VK_NULL_HANDLE;
  other.m_format = VK_FORMAT_UNDEFINED;
  other.m_width = 0;
  other.m_height = 0;
}

VulkanTexture2D& VulkanTexture2D::operator=(VulkanTexture2D&& other) noexcept
{
  if (this != &other)
  {
    destroy();

    m_allocator = other.m_allocator;
    m_device = other.m_device;
    m_image = other.m_image;
    m_allocation = other.m_allocation;
    m_imageView = other.m_imageView;
    m_sampler = other.m_sampler;
    m_format = other.m_format;
    m_width = other.m_width;
    m_height = other.m_height;

    other.m_allocator = nullptr;
    other.m_device = VK_NULL_HANDLE;
    other.m_image = VK_NULL_HANDLE;
    other.m_allocation = nullptr;
    other.m_imageView = VK_NULL_HANDLE;
    other.m_sampler = VK_NULL_HANDLE;
    other.m_format = VK_FORMAT_UNDEFINED;
    other.m_width = 0;
    other.m_height = 0;
  }

  return *this;
}

void VulkanTexture2D::createRGBA8(
    IContext& ctx, uint32_t width, uint32_t height, const void* pixels)
{
  if (width == 0 || height == 0)
  {
    throw std::runtime_error("VulkanTexture2D::createRGBA8(): invalid dimensions");
  }

  if (pixels == nullptr)
  {
    throw std::runtime_error("VulkanTexture2D::createRGBA8(): pixels is null");
  }

  if (m_image != VK_NULL_HANDLE)
  {
    throw std::runtime_error("VulkanTexture2D::createRGBA8(): texture already created");
  }

  m_allocator = &ctx.allocator();
  m_device = ctx.device();
  m_format = VK_FORMAT_R8G8B8A8_UNORM;
  m_width = width;
  m_height = height;

  VkImageCreateInfo imageCi{};
  imageCi.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageCi.imageType = VK_IMAGE_TYPE_2D;
  imageCi.format = m_format;
  imageCi.extent.width = width;
  imageCi.extent.height = height;
  imageCi.extent.depth = 1;
  imageCi.mipLevels = 1;
  imageCi.arrayLayers = 1;
  imageCi.samples = VK_SAMPLE_COUNT_1_BIT;
  imageCi.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageCi.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  imageCi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageCi.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  VmaAllocationCreateInfo allocCi{};
  allocCi.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

  vkCheck(
      vmaCreateImage(m_allocator->handle(), &imageCi, &allocCi, &m_image, &m_allocation, nullptr),
      "vmaCreateImage texture");

  VkImageViewCreateInfo viewCi{};
  viewCi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewCi.image = m_image;
  viewCi.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewCi.format = m_format;
  viewCi.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewCi.subresourceRange.baseMipLevel = 0;
  viewCi.subresourceRange.levelCount = 1;
  viewCi.subresourceRange.baseArrayLayer = 0;
  viewCi.subresourceRange.layerCount = 1;

  vkCheck(vkCreateImageView(m_device, &viewCi, nullptr, &m_imageView), "vkCreateImageView texture");

  VkSamplerCreateInfo samplerCi{};
  samplerCi.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerCi.magFilter = VK_FILTER_LINEAR;
  samplerCi.minFilter = VK_FILTER_LINEAR;
  samplerCi.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerCi.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerCi.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerCi.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerCi.mipLodBias = 0.0f;
  samplerCi.anisotropyEnable = VK_FALSE;
  samplerCi.maxAnisotropy = 1.0f;
  samplerCi.compareEnable = VK_FALSE;
  samplerCi.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerCi.minLod = 0.0f;
  samplerCi.maxLod = 0.0f;
  samplerCi.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerCi.unnormalizedCoordinates = VK_FALSE;

  vkCheck(vkCreateSampler(m_device, &samplerCi, nullptr, &m_sampler), "vkCreateSampler");

  ctx.uploadContext().uploadImage2D(
      &ctx.allocator(),
      pixels,
      static_cast<size_t>(width) * static_cast<size_t>(height) * 4,
      m_image,
      width,
      height);
}

void VulkanTexture2D::destroy()
{
  if (m_device != VK_NULL_HANDLE)
  {
    if (m_sampler != VK_NULL_HANDLE)
    {
      vkDestroySampler(m_device, m_sampler, nullptr);
      m_sampler = VK_NULL_HANDLE;
    }

    if (m_imageView != VK_NULL_HANDLE)
    {
      vkDestroyImageView(m_device, m_imageView, nullptr);
      m_imageView = VK_NULL_HANDLE;
    }
  }

  if (m_allocator != nullptr && m_image != VK_NULL_HANDLE && m_allocation != nullptr)
  {
    vmaDestroyImage(m_allocator->handle(), m_image, m_allocation);
  }

  m_image = VK_NULL_HANDLE;
  m_allocation = nullptr;
  m_allocator = nullptr;
  m_device = VK_NULL_HANDLE;
  m_format = VK_FORMAT_UNDEFINED;
  m_width = 0;
  m_height = 0;
}

}  // namespace eng