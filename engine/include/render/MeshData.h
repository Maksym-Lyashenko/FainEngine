#pragma once

#include "render/VertexTypes.h"

#include <cstdint>
#include <string>
#include <vector>

namespace eng
{

struct MeshData
{
  std::vector<eng::VertexP3> vertices;
  std::vector<uint32_t> indices;
};

struct MeshDataP3N3UV2
{
  std::vector<eng::VertexP3N3UV2> vertices;
  std::vector<uint32_t> indices;
};

MeshData LoadFirstMeshPositionsOnly(const std::string& path);
MeshDataP3N3UV2 LoadFirstMeshP3N3UV2(const std::string& path);

}  // namespace eng