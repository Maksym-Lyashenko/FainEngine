#include "render/RenderableStaticMesh.h"

#include "vk/VulkanCommon.h"
#include "vk/VulkanContext.h"

#include <stdexcept>

namespace eng
{
void RenderableStaticMesh::create(
    IContext& ctx,
    ShaderModuleHandle vert,
    ShaderModuleHandle frag,
    const SceneUniforms& sceneUniforms,
    const TextureCube& environmentCube,
    std::span<const Vertex>
        vertices,
    std::span<const Index>
        indices,
    const char* albedoTexturePath)
{
  if (albedoTexturePath == nullptr || albedoTexturePath[0] == '\0')
  {
    throw std::runtime_error("RenderableStaticMesh::create(): invalid albedo texture path");
  }

  if (vertices.empty())
  {
    throw std::runtime_error("RenderableStaticMesh::create(): vertices are empty");
  }

  if (indices.empty())
  {
    throw std::runtime_error("RenderableStaticMesh::create(): indices are empty");
  }

  m_mesh.create(ctx, vertices, indices);
  m_albedoTexture.createFromFile(ctx, albedoTexturePath);
  m_pass.create(ctx, vert, frag, sceneUniforms, m_albedoTexture, environmentCube);
}

void RenderableStaticMesh::destroy()
{
  m_pass.destroy();
  m_albedoTexture.destroy();
  m_mesh.destroy();
}

void RenderableStaticMesh::drawSolid(ICommandBuffer& cmd) const
{
  if (!isValid())
  {
    throw std::runtime_error("RenderableStaticMesh::drawSolid(): mesh is invalid");
  }

  m_mesh.bind(cmd);
  m_pass.bindSolidPipeline(cmd);
  m_pass.bindMaterial(cmd);
  m_mesh.draw(cmd);
}

void RenderableStaticMesh::drawWireframe(ICommandBuffer& cmd) const
{
  if (!isValid())
  {
    throw std::runtime_error("RenderableStaticMesh::drawWireframe(): mesh is invalid");
  }

  m_mesh.bind(cmd);
  m_pass.bindWireframePipeline(cmd);
  m_pass.bindMaterial(cmd);
  cmd.cmdSetDepthBias(0.0f, -1.0f, 0.0f);
  m_mesh.draw(cmd);
  cmd.cmdSetDepthBias(0.0f, 0.0f, 0.0f);
}

}  // namespace eng