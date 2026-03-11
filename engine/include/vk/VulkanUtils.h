#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <sstream>

namespace eng
{

struct Result
{
  enum class Code
  {
    Ok,
    ArgumentOutOfRange,
    RuntimeError,
  };

  Code code = Code::Ok;
  const char* message = "";

  Result() = default;
  Result(Code inCode, const char* inMessage = "") : code(inCode), message(inMessage) {}

  bool isOk() const { return code == Code::Ok; }
};

constexpr std::uint32_t calcNumMipLevels(std::uint32_t width, std::uint32_t height)
{
  std::uint32_t levels = 1;

  while ((width | height) >> levels)
  {
    levels++;
  }

  return levels;
}

inline VkFormat FindSupportedDepthFormat(VkPhysicalDevice physicalDevice)
{
  const VkFormat candidates[] = {
      VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};

  for (VkFormat f : candidates)
  {
    VkFormatProperties props{};
    vkGetPhysicalDeviceFormatProperties(physicalDevice, f, &props);
    if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
      return f;
    }
  }
  throw std::runtime_error("No supported depth format");
}

}  // namespace eng