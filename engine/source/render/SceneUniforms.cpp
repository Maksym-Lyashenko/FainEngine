#include "render/SceneUniforms.h"

#include "vk/VulkanBufferUtils.h"
#include "vk/VulkanContext.h"

#include <stdexcept>

namespace eng
{
void SceneUniforms::create(IContext& ctx)
{
  if (m_buffer.isValid())
  {
    throw std::runtime_error("SceneUniforms::create(): already created");
  }

  m_ctx = &ctx;
  m_buffer = CreateUniformBuffer<PerFrameData>(ctx);
}

void SceneUniforms::destroy()
{
  m_buffer.destroy();
  m_ctx = nullptr;
}

void SceneUniforms::update(const PerFrameData& data)
{
  if (!m_buffer.isValid())
  {
    throw std::runtime_error("SceneUniforms::update(): buffer is not created");
  }

  m_buffer.upload(&data, sizeof(data));
}

}  // namespace eng