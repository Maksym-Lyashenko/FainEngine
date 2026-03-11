#pragma once

#include <eng.h>

class Game : public eng::Application
{
 public:
  bool Init() override;
  void Update(float DeltaTime) override;
  void Destroy() override;

 private:
  struct Vertex
  {
    float position[2];
    float color[3];
  };

 private:
  eng::RenderPipelineHandle m_trianglePipeline = eng::kInvalidHandle;
  eng::VulkanBuffer m_vertexBuffer;
};