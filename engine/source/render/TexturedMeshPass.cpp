#include "render/TexturedMeshPass.h"

#include "graphics/GraphicsPipelineBuilder.h"
#include "vk/VulkanCommon.h"
#include "vk/VulkanContext.h"
#include "render/VertexTypes.h"

#include <array>
#include <stdexcept>

namespace eng
{
void TexturedMeshPass::create(
    IContext& ctx,
    ShaderModuleHandle vert,
    ShaderModuleHandle frag,
    const SceneUniforms& sceneUniforms,
    const Texture2D& albedoTexture,
    const TextureCube& environmentCube)
{
  if (!sceneUniforms.isValid())
  {
    throw std::runtime_error("TexturedMeshPass::create(): scene uniforms are invalid");
  }

  if (!albedoTexture.isValid())
  {
    throw std::runtime_error("TexturedMeshPass::create(): albedo texture is invalid");
  }

  if (!environmentCube.isValid())
  {
    throw std::runtime_error("TexturedMeshPass::create(): environment cube is invalid");
  }

  m_bindings.create(
      ctx, sceneUniforms.handle(), sceneUniforms.size(), albedoTexture, environmentCube);

  m_pipelineSolid = GraphicsPipelineBuilder(ctx)
                        .Shaders(vert, frag)
                        .VertexLayout<VertexP3N3UV2>()
                        .DescriptorSetLayout(m_bindings.layout())
                        .CullBack()
                        .FillMode(VK_POLYGON_MODE_FILL)
                        .Depth(ctx.getDepthFormat(), true, true, VK_COMPARE_OP_LESS)
                        .Build();

  const uint32_t isWireframe = 1;
  const std::array<VkSpecializationMapEntry, 1> specEntries = {VkSpecializationMapEntry{
      .constantID = 0,
      .offset = 0,
      .size = sizeof(uint32_t),
  }};

  m_pipelineWireframe =
      GraphicsPipelineBuilder(ctx)
          .Shaders(vert, frag)
          .VertexLayout<VertexP3N3UV2>()
          .DescriptorSetLayout(m_bindings.layout())
          .CullBack()
          .FillMode(VK_POLYGON_MODE_LINE)
          .Depth(ctx.getDepthFormat(), true, false, VK_COMPARE_OP_LESS_OR_EQUAL)
          .Specialization(
              VK_SHADER_STAGE_FRAGMENT_BIT, specEntries, &isWireframe, sizeof(isWireframe))
          .Build();

  if (!m_pipelineSolid.valid() || !m_pipelineWireframe.valid())
  {
    throw std::runtime_error("TexturedMeshPass::create(): failed to create pipeline(s)");
  }
}

void TexturedMeshPass::destroy()
{
  m_bindings.destroy();
  m_pipelineSolid.reset();
  m_pipelineWireframe.reset();
}

void TexturedMeshPass::bindMaterial(ICommandBuffer& cmd) const
{
  if (!m_bindings.isValid())
  {
    throw std::runtime_error("TexturedMeshPass::bindMaterial(): bindings are invalid");
  }

  m_bindings.bind(cmd);
}

void TexturedMeshPass::bindSolidPipeline(ICommandBuffer& cmd) const
{
  if (!m_pipelineSolid.valid())
  {
    throw std::runtime_error("TexturedMeshPass::bindSolidPipeline(): solid pipeline is invalid");
  }

  cmd.cmdBindRenderPipeline(m_pipelineSolid.get());
}

void TexturedMeshPass::bindWireframePipeline(ICommandBuffer& cmd) const
{
  if (!m_pipelineWireframe.valid())
  {
    throw std::runtime_error(
        "TexturedMeshPass::bindWireframePipeline(): wireframe pipeline is invalid");
  }

  cmd.cmdBindRenderPipeline(m_pipelineWireframe.get());
}

}  // namespace eng