#pragma once

#include "render/SceneUniforms.h"
#include "render/TexturedMeshPass.h"
#include "graphics/Texture2D.h"
#include "render/VertexTypes.h"
#include "render/StaticMesh.h"

#include <span>

namespace eng
{
class IContext;
class ICommandBuffer;

class RenderableStaticMesh
{
 public:
  using Vertex = VertexP3N3UV2;
  using Index = uint32_t;

 public:
  RenderableStaticMesh() = default;
  ~RenderableStaticMesh() = default;

  RenderableStaticMesh(const RenderableStaticMesh&) = delete;
  RenderableStaticMesh& operator=(const RenderableStaticMesh&) = delete;

  RenderableStaticMesh(RenderableStaticMesh&&) noexcept = default;
  RenderableStaticMesh& operator=(RenderableStaticMesh&&) noexcept = default;

  void create(
      IContext& ctx,
      ShaderModuleHandle vert,
      ShaderModuleHandle frag,
      const SceneUniforms& sceneUniforms,
      const TextureCube& environmentCube,
      std::span<const Vertex>
          vertices,
      std::span<const Index>
          indices,
      const char* albedoTexturePath);

  void destroy();

  bool isValid() const { return m_mesh.isValid() && m_albedoTexture.isValid() && m_pass.isValid(); }

  void drawSolid(ICommandBuffer& cmd) const;
  void drawWireframe(ICommandBuffer& cmd) const;

  const StaticMesh<Vertex, Index>& mesh() const { return m_mesh; }
  StaticMesh<Vertex, Index>& mesh() { return m_mesh; }

  const Texture2D& albedoTexture() const { return m_albedoTexture; }
  Texture2D& albedoTexture() { return m_albedoTexture; }

 private:
  StaticMesh<Vertex, Index> m_mesh;
  Texture2D m_albedoTexture;
  TexturedMeshPass m_pass;
};

}  // namespace eng