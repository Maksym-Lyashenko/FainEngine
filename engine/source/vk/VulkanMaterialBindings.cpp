#include "vk/VulkanMaterialBindings.h"

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

VulkanMaterialBindings::~VulkanMaterialBindings()
{
  destroy();
}

VulkanMaterialBindings::VulkanMaterialBindings(VulkanMaterialBindings&& other) noexcept
    : m_device(other.m_device), m_layout(other.m_layout), m_pool(other.m_pool), m_set(other.m_set)
{
  other.m_device = VK_NULL_HANDLE;
  other.m_layout = VK_NULL_HANDLE;
  other.m_pool = VK_NULL_HANDLE;
  other.m_set = VK_NULL_HANDLE;
}

VulkanMaterialBindings& VulkanMaterialBindings::operator=(VulkanMaterialBindings&& other) noexcept
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

void VulkanMaterialBindings::create(
    IContext& ctx, VkBuffer uniformBuffer, VkDeviceSize uniformSize, const Texture2D& texture)
{
  if (uniformBuffer == VK_NULL_HANDLE || uniformSize == 0)
  {
    throw std::runtime_error("VulkanMaterialBindings::create(): invalid uniform buffer");
  }

  if (!texture.isValid())
  {
    throw std::runtime_error("VulkanMaterialBindings::create(): invalid texture");
  }

  if (m_layout != VK_NULL_HANDLE || m_pool != VK_NULL_HANDLE || m_set != VK_NULL_HANDLE)
  {
    throw std::runtime_error("VulkanMaterialBindings::create(): already created");
  }

  m_device = ctx.device();

  VkDescriptorSetLayoutBinding bindings[2]{};

  bindings[0].binding = 0;
  bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  bindings[0].descriptorCount = 1;
  bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

  bindings[1].binding = 1;
  bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  bindings[1].descriptorCount = 1;
  bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutCreateInfo layoutCi{};
  layoutCi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutCi.bindingCount = 2;
  layoutCi.pBindings = bindings;

  vkCheck(
      vkCreateDescriptorSetLayout(m_device, &layoutCi, nullptr, &m_layout),
      "vkCreateDescriptorSetLayout");

  VkDescriptorPoolSize poolSizes[2]{};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = 1;
  poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[1].descriptorCount = 1;

  VkDescriptorPoolCreateInfo poolCi{};
  poolCi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolCi.maxSets = 1;
  poolCi.poolSizeCount = 2;
  poolCi.pPoolSizes = poolSizes;

  vkCheck(vkCreateDescriptorPool(m_device, &poolCi, nullptr, &m_pool), "vkCreateDescriptorPool");

  VkDescriptorSetAllocateInfo alloc{};
  alloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc.descriptorPool = m_pool;
  alloc.descriptorSetCount = 1;
  alloc.pSetLayouts = &m_layout;

  vkCheck(vkAllocateDescriptorSets(m_device, &alloc, &m_set), "vkAllocateDescriptorSets");

  VkDescriptorBufferInfo bufferInfo{};
  bufferInfo.buffer = uniformBuffer;
  bufferInfo.offset = 0;
  bufferInfo.range = uniformSize;

  VkDescriptorImageInfo imageInfo{};
  imageInfo.sampler = texture.resource().sampler();
  imageInfo.imageView = texture.resource().imageView();
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  VkWriteDescriptorSet writes[2]{};

  writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writes[0].dstSet = m_set;
  writes[0].dstBinding = 0;
  writes[0].descriptorCount = 1;
  writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  writes[0].pBufferInfo = &bufferInfo;

  writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writes[1].dstSet = m_set;
  writes[1].dstBinding = 1;
  writes[1].descriptorCount = 1;
  writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  writes[1].pImageInfo = &imageInfo;

  vkUpdateDescriptorSets(m_device, 2, writes, 0, nullptr);
}

void VulkanMaterialBindings::create(
    IContext& ctx,
    VkBuffer uniformBuffer,
    VkDeviceSize uniformSize,
    const Texture2D& texture,
    const TextureCube& cubeTexture)
{
  if (uniformBuffer == VK_NULL_HANDLE || uniformSize == 0)
  {
    throw std::runtime_error("VulkanMaterialBindings::create(): invalid uniform buffer");
  }

  if (!texture.isValid())
  {
    throw std::runtime_error("VulkanMaterialBindings::create(): invalid texture");
  }

  if (m_layout != VK_NULL_HANDLE || m_pool != VK_NULL_HANDLE || m_set != VK_NULL_HANDLE)
  {
    throw std::runtime_error("VulkanMaterialBindings::create(): already created");
  }

  m_device = ctx.device();

  VkDescriptorSetLayoutBinding bindings[3]{};

  bindings[0].binding = 0;
  bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  bindings[0].descriptorCount = 1;
  bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

  bindings[1].binding = 1;
  bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  bindings[1].descriptorCount = 1;
  bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  bindings[2].binding = 2;
  bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  bindings[2].descriptorCount = 1;
  bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutCreateInfo layoutCi{};
  layoutCi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutCi.bindingCount = 3;
  layoutCi.pBindings = bindings;

  vkCheck(
      vkCreateDescriptorSetLayout(m_device, &layoutCi, nullptr, &m_layout),
      "vkCreateDescriptorSetLayout");

  VkDescriptorPoolSize poolSizes[3]{};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = 1;

  poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[1].descriptorCount = 1;

  poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[2].descriptorCount = 1;

  VkDescriptorPoolCreateInfo poolCi{};
  poolCi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolCi.maxSets = 1;
  poolCi.poolSizeCount = 3;
  poolCi.pPoolSizes = poolSizes;

  vkCheck(vkCreateDescriptorPool(m_device, &poolCi, nullptr, &m_pool), "vkCreateDescriptorPool");

  VkDescriptorSetAllocateInfo alloc{};
  alloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc.descriptorPool = m_pool;
  alloc.descriptorSetCount = 1;
  alloc.pSetLayouts = &m_layout;

  vkCheck(vkAllocateDescriptorSets(m_device, &alloc, &m_set), "vkAllocateDescriptorSets");

  VkDescriptorBufferInfo bufferInfo{};
  bufferInfo.buffer = uniformBuffer;
  bufferInfo.offset = 0;
  bufferInfo.range = uniformSize;

  VkDescriptorImageInfo imageInfo{};
  imageInfo.sampler = texture.resource().sampler();
  imageInfo.imageView = texture.resource().imageView();
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  VkDescriptorImageInfo cubeInfo{};
  cubeInfo.sampler = cubeTexture.sampler();
  cubeInfo.imageView = cubeTexture.imageView();
  cubeInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  VkWriteDescriptorSet writes[3]{};

  writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writes[0].dstSet = m_set;
  writes[0].dstBinding = 0;
  writes[0].descriptorCount = 1;
  writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  writes[0].pBufferInfo = &bufferInfo;

  writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writes[1].dstSet = m_set;
  writes[1].dstBinding = 1;
  writes[1].descriptorCount = 1;
  writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  writes[1].pImageInfo = &imageInfo;

  writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writes[2].dstSet = m_set;
  writes[2].dstBinding = 2;
  writes[2].descriptorCount = 1;
  writes[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  writes[2].pImageInfo = &cubeInfo;

  vkUpdateDescriptorSets(m_device, 3, writes, 0, nullptr);
}

void VulkanMaterialBindings::destroy()
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

void VulkanMaterialBindings::bind(ICommandBuffer& cmd, uint32_t setIndex) const
{
  if (m_set == VK_NULL_HANDLE)
  {
    throw std::runtime_error("VulkanMaterialBindings::bind(): invalid descriptor set");
  }

  cmd.cmdBindDescriptorSet(m_set, setIndex);
}

}  // namespace eng