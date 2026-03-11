#pragma once

#include <vulkan/vulkan.h>

#include <array>
#include <cstdint>

namespace eng
{
constexpr uint32_t kInvalidHandle = 0xFFFFFFFFu;

using TextureHandle = uint32_t;
using ShaderModuleHandle = uint32_t;
using RenderPipelineHandle = uint32_t;

enum class LoadOp : uint8_t
{
  Load,
  Clear,
  DontCare
};

struct ColorAttachmentDesc
{
  LoadOp loadOp = LoadOp::Load;
  std::array<float, 4> clearColor{0.0f, 0.0f, 0.0f, 1.0f};
};

struct BeginRenderingDesc
{
  std::array<ColorAttachmentDesc, 1> color{};
};

struct RenderingTargets
{
  std::array<TextureHandle, 1> color{kInvalidHandle};
};

class ICommandBuffer
{
 public:
  virtual ~ICommandBuffer() = default;

  virtual void cmdBeginRendering(
      const BeginRenderingDesc& desc, const RenderingTargets& targets) = 0;
  virtual void cmdBindRenderPipeline(RenderPipelineHandle pipeline) = 0;
  virtual void cmdBindVertexBuffer(VkBuffer buffer, VkDeviceSize offset = 0) = 0;
  virtual void cmdDraw(
      uint32_t vertexCount,
      uint32_t instanceCount = 1,
      uint32_t firstVertex = 0,
      uint32_t firstInstance = 0) = 0;
  virtual void cmdPushDebugGroupLabel(const char* name, uint32_t rgba8) = 0;
  virtual void cmdPopDebugGroupLabel() = 0;
  virtual void cmdEndRendering() = 0;
};

}  // namespace eng