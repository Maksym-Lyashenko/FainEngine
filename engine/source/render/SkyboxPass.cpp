#include "render/SkyboxPass.h"

#include "graphics/GraphicsPipelineBuilder.h"
#include "vk/VulkanCommon.h"
#include "vk/VulkanContext.h"

#include <stdexcept>

namespace eng
{
void SkyboxPass::create(
    IContext& ctx,
    ShaderModuleHandle vert,
    ShaderModuleHandle frag,
    const SceneUniforms& sceneUniforms,
    const TextureCube& cubeTexture)
{
  if (!sceneUniforms.isValid())
  {
    throw std::runtime_error("SkyboxPass::create(): scene uniforms are invalid");
  }

  if (!cubeTexture.isValid())
  {
    throw std::runtime_error("SkyboxPass::create(): cube texture is invalid");
  }

  m_bindings.create(ctx, sceneUniforms.handle(), sceneUniforms.size(), cubeTexture);

  m_pipeline = GraphicsPipelineBuilder(ctx)
                   .Shaders(vert, frag)
                   .DescriptorSetLayout(m_bindings.layout())
                   .Depth(ctx.getDepthFormat(), true, false, VK_COMPARE_OP_LESS_OR_EQUAL)
                   .Build();

  if (!m_pipeline.valid())
  {
    throw std::runtime_error("SkyboxPass::create(): failed to create pipeline");
  }
}

void SkyboxPass::destroy()
{
  m_bindings.destroy();
  m_pipeline.reset();
}

void SkyboxPass::draw(ICommandBuffer& cmd) const
{
  if (!isValid())
  {
    throw std::runtime_error("SkyboxPass::draw(): pass is invalid");
  }

  cmd.cmdBindRenderPipeline(m_pipeline.get());
  m_bindings.bind(cmd);
  cmd.cmdDraw(36);
}

}  // namespace eng