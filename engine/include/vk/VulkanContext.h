#pragma once

#include "vk/VulkanCommon.h"
#include "vk/VulkanPipelineCache.h"
#include "vk/VulkanAllocator.h"
#include "vk/VulkanUploadContext.h"

#include <memory>
#include <string>
#include <vector>

struct GLFWwindow;

namespace eng
{
struct ContextCreateInfo
{
  bool enableValidation = true;
  uint32_t framesInFlight = 2;
  VkPresentModeKHR preferredPresentMode = VK_PRESENT_MODE_FIFO_KHR;
  VkFormat preferredColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
};

class IContext
{
 public:
  virtual ~IContext() = default;

  virtual ICommandBuffer& acquireCommandBuffer() = 0;
  virtual void submit(ICommandBuffer& cmd, TextureHandle presentTexture) = 0;
  virtual void waitIdle() = 0;

  virtual ShaderModuleHandle createShaderModule(const ShaderModuleDesc& desc) = 0;
  virtual void destroyShaderModule(ShaderModuleHandle handle) = 0;

  virtual RenderPipelineHandle createRenderPipeline(const RenderPipelineDesc& desc) = 0;
  virtual void destroyRenderPipeline(RenderPipelineHandle handle) = 0;

  virtual VulkanAllocator& allocator() = 0;
  virtual VulkanUploadContext& uploadContext() = 0;

  virtual VkFormat getSwapchainFormat() const = 0;
  virtual VkExtent2D getSwapchainExtent() const = 0;
  virtual TextureHandle getCurrentSwapchainTexture() const = 0;

  virtual VkFormat getDepthFormat() const = 0;
};

std::unique_ptr<IContext> createVulkanContextWithSwapchain(
    GLFWwindow* window, const ContextCreateInfo& ci = {});

std::vector<uint32_t> readSpirvFile(const std::string& path);

inline ShaderModuleHandle loadShaderModule(IContext* ctx, const std::string& spvPath)
{
  const std::vector<uint32_t> code = readSpirvFile(spvPath);

  ShaderModuleDesc desc{};
  desc.code = code.data();
  desc.sizeByte = code.size() * sizeof(uint32_t);

  return ctx->createShaderModule(desc);
}

}  // namespace eng