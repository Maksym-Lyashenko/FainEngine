#pragma once

#include "vk/VulkanContext.h"
#include "vk/VulkanDescriptors.h"
#include "vk/VulkanTexture2D.h"

#include <string>

namespace eng
{
class Texture2D
{
 public:
  Texture2D() = default;
  ~Texture2D() = default;

  Texture2D(const Texture2D&) = delete;
  Texture2D& operator=(const Texture2D&) = delete;

  Texture2D(Texture2D&&) noexcept = default;
  Texture2D& operator=(Texture2D&&) noexcept = default;

  void createFromRGBA8(IContext& ctx, uint32_t width, uint32_t height, const void* pixels);

  void createFromFile(IContext& ctx, const std::string& path);

  void destroy();

  bool isValid() const;

  void bind(ICommandBuffer& cmd, uint32_t setIndex = 0) const;

  VkDescriptorSetLayout descriptorSetLayout() const;
  VkDescriptorSet descriptorSet() const;

  uint32_t width() const;
  uint32_t height() const;

  const VulkanTexture2D& resource() const { return m_texture; }
  VulkanTexture2D& resource() { return m_texture; }

 private:
  VulkanTexture2D m_texture;
  VulkanCombinedImageSamplerBindings m_bindings;
};

}  // namespace eng