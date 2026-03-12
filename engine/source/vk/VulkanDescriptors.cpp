#include "vk/VulkanDescriptors.h"

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

VulkanCombinedImageSamplerBindings::~VulkanCombinedImageSamplerBindings()
{
  destroy();
}

VulkanCombinedImageSamplerBindings::VulkanCombinedImageSamplerBindings(
    VulkanCombinedImageSamplerBindings&& other) noexcept
    : m_device(other.m_device), m_layout(other.m_layout), m_pool(other.m_pool), m_set(other.m_set)
{
  other.m_device = VK_NULL_HANDLE;
  other.m_layout = VK_NULL_HANDLE;
  other.m_pool = VK_NULL_HANDLE;
  other.m_set = VK_NULL_HANDLE;
}

VulkanCombinedImageSamplerBindings& VulkanCombinedImageSamplerBindings::operator=(
    VulkanCombinedImageSamplerBindings&& other) noexcept
{
  if (this != &other)
  {
    destroy();

    m_device = other.m_device;
    m_layout = other.m_layout;
    m_pool = other.m_pool;
    m_set = other.m_set;

    other.m_device = VK_NULL_HANDLE;
    other.m_layout = VK_NULL_HANDLE;
    other.m_pool = VK_NULL_HANDLE;
    other.m_set = VK_NULL_HANDLE;
  }

  return *this;
}

void VulkanCombinedImageSamplerBindings::create(
    IContext& ctx, VkImageView imageView, VkSampler sampler)
{
  if (imageView == VK_NULL_HANDLE || sampler == VK_NULL_HANDLE)
  {
    throw std::runtime_error(
        "VulkanCombinedImageSamplerBindings::create(): invalid texture view/sampler");
  }

  if (m_layout != VK_NULL_HANDLE || m_pool != VK_NULL_HANDLE || m_set != VK_NULL_HANDLE)
  {
    throw std::runtime_error("VulkanCombinedImageSamplerBindings::create(): already created");
  }

  m_device = ctx.device();

  VkDescriptorSetLayoutBinding binding{};
  binding.binding = 0;
  binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  binding.descriptorCount = 1;
  binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutCreateInfo layoutCi{};
  layoutCi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutCi.bindingCount = 1;
  layoutCi.pBindings = &binding;

  vkCheck(
      vkCreateDescriptorSetLayout(m_device, &layoutCi, nullptr, &m_layout),
      "vkCreateDescriptorSetLayout");

  VkDescriptorPoolSize poolSize{};
  poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSize.descriptorCount = 1;

  VkDescriptorPoolCreateInfo poolCi{};
  poolCi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolCi.maxSets = 1;
  poolCi.poolSizeCount = 1;
  poolCi.pPoolSizes = &poolSize;

  vkCheck(vkCreateDescriptorPool(m_device, &poolCi, nullptr, &m_pool), "vkCreateDescriptorPool");

  VkDescriptorSetAllocateInfo alloc{};
  alloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc.descriptorPool = m_pool;
  alloc.descriptorSetCount = 1;
  alloc.pSetLayouts = &m_layout;

  vkCheck(vkAllocateDescriptorSets(m_device, &alloc, &m_set), "vkAllocateDescriptorSets");

  VkDescriptorImageInfo imageInfo{};
  imageInfo.sampler = sampler;
  imageInfo.imageView = imageView;
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = m_set;
  write.dstBinding = 0;
  write.dstArrayElement = 0;
  write.descriptorCount = 1;
  write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  write.pImageInfo = &imageInfo;

  vkUpdateDescriptorSets(m_device, 1, &write, 0, nullptr);
}

void VulkanCombinedImageSamplerBindings::destroy()
{
  if (m_device != VK_NULL_HANDLE)
  {
    if (m_pool != VK_NULL_HANDLE)
    {
      vkDestroyDescriptorPool(m_device, m_pool, nullptr);
      m_pool = VK_NULL_HANDLE;
    }

    if (m_layout != VK_NULL_HANDLE)
    {
      vkDestroyDescriptorSetLayout(m_device, m_layout, nullptr);
      m_layout = VK_NULL_HANDLE;
    }
  }

  m_set = VK_NULL_HANDLE;
  m_device = VK_NULL_HANDLE;
}

}  // namespace eng