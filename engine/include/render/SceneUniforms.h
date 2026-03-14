#pragma once

#include "render/PerFrameData.h"
#include "vk/VulkanBuffer.h"

namespace eng
{
class IContext;

class SceneUniforms
{
 public:
  SceneUniforms() = default;
  ~SceneUniforms() = default;

  SceneUniforms(const SceneUniforms&) = delete;
  SceneUniforms& operator=(const SceneUniforms&) = delete;

  SceneUniforms(SceneUniforms&&) noexcept = default;
  SceneUniforms& operator=(SceneUniforms&&) noexcept = default;

  void create(IContext& ctx);
  void destroy();

  void update(const PerFrameData& data);

  bool isValid() const { return m_buffer.isValid(); }

  VkBuffer handle() const { return m_buffer.handle(); }
  VkDeviceSize size() const { return sizeof(PerFrameData); }

  const VulkanBuffer& buffer() const { return m_buffer; }
  VulkanBuffer& buffer() { return m_buffer; }

 private:
  IContext* m_ctx = nullptr;
  VulkanBuffer m_buffer;
};

}  // namespace eng