#pragma once

#include "eng.h"

class Game final : public eng::Application
{
 public:
  bool Init() override;
  void Update(float DeltaTime) override;
  void Destroy() override;

 private:
  eng::ImGuiLayer m_imgui;
  eng::Texture2D m_texture;
  ImTextureID m_imguiTexture{};
};