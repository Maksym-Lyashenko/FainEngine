#pragma once

#include "graphics/ShaderModule.h"
#include "render/SceneUniforms.h"
#include "graphics/TextureCube.h"
#include "vk/VulkanSkyboxBindings.h"
#include "vk/VulkanResourceHolder.h"

namespace eng
{
class IContext;
class ICommandBuffer;

class SkyboxPass
{
 public:
  SkyboxPass() = default;
  ~SkyboxPass() = default;

  SkyboxPass(const SkyboxPass&) = delete;
  SkyboxPass& operator=(const SkyboxPass&) = delete;

  SkyboxPass(SkyboxPass&&) noexcept = default;
  SkyboxPass& operator=(SkyboxPass&&) noexcept = default;

  void create(
      IContext& ctx,
      ShaderModuleHandle vert,
      ShaderModuleHandle frag,
      const SceneUniforms& sceneUniforms,
      const TextureCube& cubeTexture);

  void destroy();

  bool isValid() const { return m_pipeline.valid() && m_bindings.isValid(); }

  void draw(ICommandBuffer& cmd) const;

 private:
  Holder<RenderPipelineHandle> m_pipeline;
  VulkanSkyboxBindings m_bindings;
};

}  // namespace eng