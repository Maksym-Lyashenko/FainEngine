#pragma once

#include "eng.h"

struct GLFWwindow;

class Game : public eng::Application
{
 public:
  bool Init() override;
  void Update(float DeltaTime) override;
  void Destroy() override;

 private:
  
};