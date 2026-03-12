#pragma once

#include <vulkan/vulkan.h>

#include <array>
#include <cstdint>

namespace eng
{
constexpr uint32_t kInvalidHandle = 0xFFFFFFFFu;

struct TextureHandle
{
  uint32_t id = kInvalidHandle;

  constexpr TextureHandle() noexcept = default;
  constexpr TextureHandle(uint32_t value) noexcept : id(value) {}

  constexpr bool valid() const noexcept { return id != kInvalidHandle; }
  constexpr explicit operator bool() const noexcept { return valid(); }
  constexpr operator uint32_t() const noexcept { return id; }

  auto operator<=>(const TextureHandle&) const = default;
};

struct ShaderModuleHandle
{
  uint32_t id = kInvalidHandle;

  constexpr ShaderModuleHandle() noexcept = default;
  constexpr ShaderModuleHandle(uint32_t value) noexcept : id(value) {}

  constexpr bool valid() const noexcept { return id != kInvalidHandle; }
  constexpr explicit operator bool() const noexcept { return valid(); }
  constexpr operator uint32_t() const noexcept { return id; }

  auto operator<=>(const ShaderModuleHandle&) const = default;
};

struct RenderPipelineHandle
{
  uint32_t id = kInvalidHandle;

  constexpr RenderPipelineHandle() noexcept = default;
  constexpr RenderPipelineHandle(uint32_t value) noexcept : id(value) {}

  constexpr bool valid() const noexcept { return id != kInvalidHandle; }
  constexpr explicit operator bool() const noexcept { return valid(); }
  constexpr operator uint32_t() const noexcept { return id; }

  auto operator<=>(const RenderPipelineHandle&) const = default;
};

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

struct DepthAttachmentDesc
{
  LoadOp loadOp = LoadOp::Clear;
  float clearDepth = 1.0f;
};

struct BeginRenderingDesc
{
  std::array<ColorAttachmentDesc, 1> color{};
  bool useDepth = false;
  DepthAttachmentDesc depth{};
};

struct RenderingTargets
{
  std::array<TextureHandle, 1> color{kInvalidHandle};
};

class ICommandBuffer
{
 public:
  virtual ~ICommandBuffer() = default;

  virtual VkCommandBuffer handle() const = 0;

  virtual void cmdBeginRendering(
      const BeginRenderingDesc& desc, const RenderingTargets& targets) = 0;
  virtual void cmdBindRenderPipeline(RenderPipelineHandle pipeline) = 0;
  virtual void cmdBindDescriptorSet(VkDescriptorSet set, uint32_t setIndex = 0) = 0;
  virtual void cmdBindVertexBuffer(VkBuffer buffer, VkDeviceSize offset = 0) = 0;
  virtual void cmdBindIndexBuffer(
      VkBuffer buffer, VkDeviceSize offset = 0, VkIndexType indexType = VK_INDEX_TYPE_UINT32) = 0;

  virtual void cmdPushConstants(const void* data, uint32_t size, uint32_t offset = 0) = 0;

  template <typename T>
  void cmdPushConstants(const T& value, uint32_t offset = 0)
  {
    cmdPushConstants(&value, static_cast<uint32_t>(sizeof(T)), offset);
  }

  virtual void cmdDraw(
      uint32_t vertexCount,
      uint32_t instanceCount = 1,
      uint32_t firstVertex = 0,
      uint32_t firstInstance = 0) = 0;

  virtual void cmdDrawIndexed(
      uint32_t indexCount,
      uint32_t instanceCount = 1,
      uint32_t firstIndex = 0,
      int32_t vertexOffset = 0,
      uint32_t firstInstance = 0) = 0;

  virtual void cmdSetDepthBias(float constantFactor, float slopeFactor, float clamp = 0.0f) = 0;

  virtual void cmdPushDebugGroupLabel(const char* name, uint32_t rgba8) = 0;
  virtual void cmdPopDebugGroupLabel() = 0;
  virtual void cmdEndRendering() = 0;
};

}  // namespace eng