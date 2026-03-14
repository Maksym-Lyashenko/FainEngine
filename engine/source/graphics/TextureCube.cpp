#include "graphics/TextureCube.h"

#include <stdexcept>

namespace eng
{
void TextureCube::createFromRGBA32F(
    IContext& ctx, uint32_t faceWidth, uint32_t faceHeight, const void* pixels, size_t sizeBytes)
{
  if (pixels == nullptr || sizeBytes == 0)
  {
    throw std::runtime_error("TextureCube::createFromRGBA32F(): invalid input data");
  }

  if (isValid())
  {
    throw std::runtime_error("TextureCube::createFromRGBA32F(): already created");
  }

  m_texture.createCubeRGBA32F(ctx, faceWidth, faceHeight, pixels, sizeBytes);
}

void TextureCube::destroy()
{
  m_texture.destroy();
}

bool TextureCube::isValid() const
{
  return m_texture.isValid();
}

}  // namespace eng