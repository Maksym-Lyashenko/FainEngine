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

MeshDataP3N3UV2 LoadFirstMeshP3N3UV2(const std::string& path)
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

  if (!mesh->HasPositions())
  {
    aiReleaseImport(scene);
    throw std::runtime_error("Mesh has no positions: " + path);
  }

  if (!mesh->HasNormals())
  {
    aiReleaseImport(scene);
    throw std::runtime_error("Mesh has no normals: " + path);
  }

  MeshDataP3N3UV2 out{};
  out.vertices.reserve(mesh->mNumVertices);
  out.indices.reserve(mesh->mNumFaces * 3);

  for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
  {
    const aiVector3D& p = mesh->mVertices[i];
    const aiVector3D& n = mesh->mNormals[i];

    glm::vec2 uv{0.0f, 0.0f};
    if (mesh->HasTextureCoords(0))
    {
      const aiVector3D& t = mesh->mTextureCoords[0][i];
      uv = {t.x, t.y};
    }

    out.vertices.push_back(
        VertexP3N3UV2{
            .position = {p.x, p.y, p.z},
            .normal = {n.x, n.y, n.z},
            .uv = uv,
        });
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