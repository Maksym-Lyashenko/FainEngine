#include "render/MeshData.h"

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <stdexcept>

namespace eng
{

MeshData LoadFirstMeshPositionsOnly(const std::string& path)
{
  const aiScene* scene = aiImportFile(path.c_str(), aiProcess_Triangulate);

  if (scene == nullptr || !scene->HasMeshes())
  {
    throw std::runtime_error("Failed to load mesh: " + path);
  }

  const aiMesh* mesh = scene->mMeshes[0];
  if (mesh == nullptr)
  {
    aiReleaseImport(scene);
    throw std::runtime_error("Mesh is null: " + path);
  }

  MeshData out{};
  out.vertices.reserve(mesh->mNumVertices);
  out.indices.reserve(mesh->mNumFaces * 3);

  for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
  {
    const aiVector3D& v = mesh->mVertices[i];
    out.vertices.push_back(eng::VertexP3{.position = {v.x, v.y, v.z}});
  }

  for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
  {
    const aiFace& face = mesh->mFaces[i];
    if (face.mNumIndices != 3)
    {
      continue;
    }

    out.indices.push_back(face.mIndices[0]);
    out.indices.push_back(face.mIndices[1]);
    out.indices.push_back(face.mIndices[2]);
  }

  aiReleaseImport(scene);
  return out;
}

}  // namespace eng