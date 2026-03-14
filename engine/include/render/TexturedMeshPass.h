#pragma once

#include "graphics/ShaderModule.h"
#include "render/SceneUniforms.h"
#include "graphics/Texture2D.h"
#include "graphics/TextureCube.h"
#include "vk/VulkanMaterialBindings.h"
#include "vk/VulkanResourceHolder.h"

namespace eng
{
class IContext;
class ICommandBuffer;

class TexturedMeshPass
{
 public:
  TexturedMeshPass() = default;
  ~TexturedMeshPass() = default;

  TexturedMeshPass(const TexturedMeshPass&) = delete;
  TexturedMeshPass& operator=(const TexturedMeshPass&) = delete;

  TexturedMeshPass(TexturedMeshPass&&) noexcept = default;
  TexturedMeshPass& operator=(TexturedMeshPass&&) noexcept = default;

  void create(
      IContext& ctx,
      ShaderModuleHandle vert,
      ShaderModuleHandle frag,
      const SceneUniforms& sceneUniforms,
      const Texture2D& albedoTexture,
      const TextureCube& environmentCube);

  void destroy();

  bool isValid() const
  {
    return m_pipelineSolid.valid() && m_pipelineWireframe.valid() && m_bindings.isValid();
  }

  void bindMaterial(ICommandBuffer& cmd) const;
  void bindSolidPipeline(ICommandBuffer& cmd) const;
  void bindWireframePipeline(ICommandBuffer& cmd) const;

 private:
  Holder<RenderPipelineHandle> m_pipelineSolid;
  Holder<RenderPipelineHandle> m_pipelineWireframe;
  VulkanMaterialBindings m_bindings;
};

}  // namespace eng