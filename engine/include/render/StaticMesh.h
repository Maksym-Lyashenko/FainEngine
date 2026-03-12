#pragma once

#include "vk/VulkanContext.h"
#include "vk/VulkanMeshUtils.h"

#include <span>
#include <type_traits>

namespace eng
{
template <typename TVertex, typename TIndex = uint32_t>
class StaticMesh
{
 public:
  StaticMesh() = default;
  ~StaticMesh() = default;

  StaticMesh(const StaticMesh&) = delete;
  StaticMesh& operator=(const StaticMesh&) = delete;

  StaticMesh(StaticMesh&&) noexcept = default;
  StaticMesh& operator=(StaticMesh&&) noexcept = default;

  void create(IContext& ctx, std::span<const TVertex> vertices, std::span<const TIndex> indices)
  {
    m_buffers = CreateMeshBuffers<TVertex, TIndex>(ctx, vertices, indices);
  }

  void destroy() { m_buffers.destroy(); }

  bool isValid() const { return m_buffers.isValid(); }

  void bind(ICommandBuffer& cmd) const { m_buffers.bind(cmd); }

  void draw(ICommandBuffer& cmd, uint32_t instanceCount = 1) const
  {
    m_buffers.draw(cmd, instanceCount);
  }

  void bindAndDraw(ICommandBuffer& cmd, uint32_t instanceCount = 1) const
  {
    m_buffers.bindAndDraw(cmd, instanceCount);
  }

  uint32_t indexCount() const { return m_buffers.indexCount; }

  const MeshBuffers<TVertex, TIndex>& buffers() const { return m_buffers; }

  MeshBuffers<TVertex, TIndex>& buffers() { return m_buffers; }

 private:
  MeshBuffers<TVertex, TIndex> m_buffers;
};

}  // namespace eng