#pragma once

#include "Application.h"
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
};