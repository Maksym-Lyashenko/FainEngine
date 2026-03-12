#pragma once

#include "vk/VulkanBufferUtils.h"

#include <span>
#include <type_traits>
#include <cstdint>

namespace eng
{
template <typename TIndex>
struct IndexTypeTraits;

template <>
struct IndexTypeTraits<uint16_t>
{
  static constexpr VkIndexType kVkType = VK_INDEX_TYPE_UINT16;
};

template <>
struct IndexTypeTraits<uint32_t>
{
  static constexpr VkIndexType kVkType = VK_INDEX_TYPE_UINT32;
};

template <typename TVertex, typename TIndex = uint32_t>
struct MeshBuffers
{
  VulkanBuffer vertexBuffer;
  VulkanBuffer indexBuffer;
  uint32_t indexCount = 0;

  bool isValid() const { return vertexBuffer.isValid() && indexBuffer.isValid() && indexCount > 0; }

  void destroy()
  {
    indexBuffer.destroy();
    vertexBuffer.destroy();
    indexCount = 0;
  }

  void bind(ICommandBuffer& cmd) const
  {
    cmd.cmdBindVertexBuffer(vertexBuffer.handle());
    cmd.cmdBindIndexBuffer(indexBuffer.handle(), 0, IndexType());
  }

  void draw(ICommandBuffer& cmd, uint32_t instanceCount = 1) const
  {
    cmd.cmdDrawIndexed(indexCount, instanceCount);
  }

  void bindAndDraw(ICommandBuffer& cmd, uint32_t instanceCount = 1) const
  {
    bind(cmd);
    draw(cmd, instanceCount);
  }

  static constexpr VkIndexType IndexType()
  {
    static_assert(std::is_same_v<TIndex, uint16_t> || std::is_same_v<TIndex, uint32_t>);
    return IndexTypeTraits<TIndex>::kVkType;
  }
};

template <typename TVertex, typename TIndex = uint32_t>
inline MeshBuffers<TVertex, TIndex> CreateMeshBuffers(
    IContext& ctx, std::span<const TVertex> vertices, std::span<const TIndex> indices)
{
  static_assert(std::is_trivially_copyable_v<TVertex>);
  static_assert(std::is_trivially_copyable_v<TIndex>);
  static_assert(std::is_same_v<TIndex, uint16_t> || std::is_same_v<TIndex, uint32_t>);

  MeshBuffers<TVertex, TIndex> mesh{};
  mesh.vertexBuffer = CreateVertexBuffer(ctx, vertices);
  mesh.indexBuffer = CreateIndexBuffer(ctx, indices);
  mesh.indexCount = static_cast<uint32_t>(indices.size());

  return mesh;
}

}  // namespace eng