#pragma once

#include "vk/VulkanTexture2D.h"

#include <string>
#include <vector>

namespace eng
{
class TextureCube
{
 public:
  TextureCube() = default;
  ~TextureCube() = default;

  TextureCube(const TextureCube&) = delete;
  TextureCube& operator=(const TextureCube&) = delete;

  TextureCube(TextureCube&&) noexcept = default;
  TextureCube& operator=(TextureCube&&) noexcept = default;

  void createFromRGBA32F(
      IContext& ctx, uint32_t faceWidth, uint32_t faceHeight, const void* pixels, size_t sizeBytes);

  void destroy();

  bool isValid() const;

  VkImageView imageView() const { return m_texture.imageView(); }
  VkSampler sampler() const { return m_texture.sampler(); }

  const VulkanTexture2D& resource() const { return m_texture; }
  VulkanTexture2D& resource() { return m_texture; }

 private:
  VulkanTexture2D m_texture;
};

}  // namespace eng