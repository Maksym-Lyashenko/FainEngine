#pragma once

#include "Application.h"
#include "eng.h"

struct GLFWwindow;

class Game final : public eng::Application
{
 public:
  bool Init() override;
  void Update(float DeltaTime) override;
  void Destroy() override;

 private:
  eng::ImGuiLayer m_imgui;
  eng::FpsCounter m_fpsCounter{.5f};
  GLFWwindow* m_window;
  eng::IContext* m_ctx;
};