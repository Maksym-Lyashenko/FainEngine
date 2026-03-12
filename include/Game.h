#pragma once

#include "eng.h"

class Game final : public eng::Application
{
 public:
  bool Init() override;
  void Update(float DeltaTime) override;
  void Destroy() override;

 private:
  eng::Holder<eng::RenderPipelineHandle> m_pipelineSolid;
  eng::Holder<eng::RenderPipelineHandle> m_pipelineWireframe;

  eng::StaticMesh<eng::VertexP3, uint32_t> m_mesh;
};