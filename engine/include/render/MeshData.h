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

MeshData LoadFirstMeshPositionsOnly(const std::string& path);

}  // namespace eng