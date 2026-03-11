#include <cassert>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <vector>

#include <vulkan/vulkan.h>

#include <ktx.h>

#include <stb_image.h>
#include <stb_image_resize2.h>

#include "vulkan/VulkanUtils.h"

static constexpr uint32_t GL_COMPRESSED_RGBA_BPTC_UNORM = 0x8E8C;

namespace
{

bool EnsureDirectoryExists(const std::filesystem::path& dirPath)
{
  std::error_code ec;

  if (std::filesystem::exists(dirPath, ec))
  {
    return std::filesystem::is_directory(dirPath, ec);
  }

  return std::filesystem::create_directories(dirPath, ec);
}

bool CompressImageToBC7KTX(const char* inputFilename, const char* outputFilename)
{
  constexpr int numChannels = 4;

  int origW = 0;
  int origH = 0;
  uint8_t* pixels = stbi_load(inputFilename, &origW, &origH, nullptr, numChannels);
  if (!pixels)
  {
    return false;
  }

  const uint32_t numMipLevels =
      eng::calcNumMipLevels(static_cast<uint32_t>(origW), static_cast<uint32_t>(origH));

  // KTX2: glInternalformat игнорируется, поэтому ставим 0.
  ktxTextureCreateInfo createInfoKTX2 = {};
  createInfoKTX2.glInternalformat = 0;
  createInfoKTX2.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
  createInfoKTX2.baseWidth = static_cast<uint32_t>(origW);
  createInfoKTX2.baseHeight = static_cast<uint32_t>(origH);
  createInfoKTX2.baseDepth = 1u;
  createInfoKTX2.numDimensions = 2u;
  createInfoKTX2.numLevels = numMipLevels;
  createInfoKTX2.numLayers = 1u;
  createInfoKTX2.numFaces = 1u;
  createInfoKTX2.isArray = KTX_FALSE;
  createInfoKTX2.generateMipmaps = KTX_FALSE;

  ktxTexture2* textureKTX2 = nullptr;
  if (ktxTexture2_Create(&createInfoKTX2, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &textureKTX2) !=
      KTX_SUCCESS)
  {
    stbi_image_free(pixels);
    return false;
  }

  int w = origW;
  int h = origH;

  for (uint32_t mip = 0; mip < numMipLevels; ++mip)
  {
    ktx_size_t offset = 0;
    if (ktxTexture_GetImageOffset(ktxTexture(textureKTX2), mip, 0, 0, &offset) != KTX_SUCCESS)
    {
      ktxTexture_Destroy(ktxTexture(textureKTX2));
      stbi_image_free(pixels);
      return false;
    }

    stbir_resize_uint8_linear(
        pixels,
        origW,
        origH,
        0,
        ktxTexture_GetData(ktxTexture(textureKTX2)) + offset,
        w,
        h,
        0,
        STBIR_RGBA);

    w = (w > 1) ? (w >> 1) : 1;
    h = (h > 1) ? (h >> 1) : 1;
  }

  if (ktxTexture2_CompressBasis(textureKTX2, 255) != KTX_SUCCESS)
  {
    ktxTexture_Destroy(ktxTexture(textureKTX2));
    stbi_image_free(pixels);
    return false;
  }

  if (ktxTexture2_TranscodeBasis(textureKTX2, KTX_TTF_BC7_RGBA, 0) != KTX_SUCCESS)
  {
    ktxTexture_Destroy(ktxTexture(textureKTX2));
    stbi_image_free(pixels);
    return false;
  }

  // KTX1 BC7 output.
  ktxTextureCreateInfo createInfoKTX1 = {};
  createInfoKTX1.glInternalformat = GL_COMPRESSED_RGBA_BPTC_UNORM;
  createInfoKTX1.vkFormat = VK_FORMAT_BC7_UNORM_BLOCK;
  createInfoKTX1.baseWidth = static_cast<uint32_t>(origW);
  createInfoKTX1.baseHeight = static_cast<uint32_t>(origH);
  createInfoKTX1.baseDepth = 1u;
  createInfoKTX1.numDimensions = 2u;
  createInfoKTX1.numLevels = numMipLevels;
  createInfoKTX1.numLayers = 1u;
  createInfoKTX1.numFaces = 1u;
  createInfoKTX1.isArray = KTX_FALSE;
  createInfoKTX1.generateMipmaps = KTX_FALSE;

  ktxTexture1* textureKTX1 = nullptr;
  if (ktxTexture1_Create(&createInfoKTX1, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &textureKTX1) !=
      KTX_SUCCESS)
  {
    ktxTexture_Destroy(ktxTexture(textureKTX2));
    stbi_image_free(pixels);
    return false;
  }

  for (uint32_t mip = 0; mip < numMipLevels; ++mip)
  {
    ktx_size_t offset1 = 0;
    ktx_size_t offset2 = 0;

    if (ktxTexture_GetImageOffset(ktxTexture(textureKTX1), mip, 0, 0, &offset1) != KTX_SUCCESS ||
        ktxTexture_GetImageOffset(ktxTexture(textureKTX2), mip, 0, 0, &offset2) != KTX_SUCCESS)
    {
      ktxTexture_Destroy(ktxTexture(textureKTX1));
      ktxTexture_Destroy(ktxTexture(textureKTX2));
      stbi_image_free(pixels);
      return false;
    }

    std::memcpy(
        ktxTexture_GetData(ktxTexture(textureKTX1)) + offset1,
        ktxTexture_GetData(ktxTexture(textureKTX2)) + offset2,
        ktxTexture_GetImageSize(ktxTexture(textureKTX1), mip));
  }

  const std::filesystem::path outPath(outputFilename);
  if (!outPath.parent_path().empty())
  {
    EnsureDirectoryExists(outPath.parent_path());
  }

  const KTX_error_code writeRes =
      ktxTexture_WriteToNamedFile(ktxTexture(textureKTX1), outputFilename);

  ktxTexture_Destroy(ktxTexture(textureKTX1));
  ktxTexture_Destroy(ktxTexture(textureKTX2));
  stbi_image_free(pixels);

  return writeRes == KTX_SUCCESS;
}

}  // namespace

bool main()
{
  if (!CompressImageToBC7KTX("assets/wood.jpg", ".cache/image.ktx"))
  {
    return false;
  }

  return true;
}