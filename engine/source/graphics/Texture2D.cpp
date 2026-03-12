#include "graphics/Texture2D.h"

#include <stb_image.h>

#include <stdexcept>

namespace eng
{
void Texture2D::createFromRGBA8(IContext& ctx, uint32_t width, uint32_t height, const void* pixels)
{
  if (pixels == nullptr)
  {
    throw std::runtime_error("Texture2D::createFromRGBA8(): pixels is null");
  }

  if (isValid())
  {
    throw std::runtime_error("Texture2D::createFromRGBA8(): texture already created");
  }

  m_texture.createRGBA8(ctx, width, height, pixels);
  m_bindings.create(ctx, m_texture.imageView(), m_texture.sampler());
}

void Texture2D::createFromFile(IContext& ctx, const std::string& path)
{
  if (isValid())
  {
    throw std::runtime_error("Texture2D::createFromFile(): texture already created");
  }

  int w = 0;
  int h = 0;
  int comp = 0;

  stbi_uc* img = stbi_load(path.c_str(), &w, &h, &comp, 4);
  if (img == nullptr || w <= 0 || h <= 0)
  {
    throw std::runtime_error("Texture2D::createFromFile(): failed to load image: " + path);
  }

  try
  {
    createFromRGBA8(ctx, static_cast<uint32_t>(w), static_cast<uint32_t>(h), img);
  }
  catch (...)
  {
    stbi_image_free(img);
    throw;
  }

  stbi_image_free(img);
}

void Texture2D::destroy()
{
  m_bindings.destroy();
  m_texture.destroy();
}

bool Texture2D::isValid() const
{
  return m_texture.isValid() && m_bindings.isValid();
}

void Texture2D::bind(ICommandBuffer& cmd, uint32_t setIndex) const
{
  if (!isValid())
  {
    throw std::runtime_error("Texture2D::bind(): texture is not valid");
  }

  cmd.cmdBindDescriptorSet(m_bindings.set(), setIndex);
}

VkDescriptorSetLayout Texture2D::descriptorSetLayout() const
{
  return m_bindings.layout();
}

VkDescriptorSet Texture2D::descriptorSet() const
{
  return m_bindings.set();
}

uint32_t Texture2D::width() const
{
  return m_texture.width();
}

uint32_t Texture2D::height() const
{
  return m_texture.height();
}

}  // namespace eng