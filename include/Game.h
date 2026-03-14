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
  struct MouseState
  {
    glm::vec2 pos{0.f};
    bool pressedLeft = false;
  };

 private:
  eng::IContext* m_ctx = nullptr;
  GLFWwindow* m_window = nullptr;

  eng::TextureCube m_environmentCube;

  eng::SceneUniforms m_sceneUniforms;
  eng::SkyboxPass m_skyboxPass;

  eng::RenderableStaticMesh m_duck;

  MouseState m_mouseState{};

  eng::CameraPositionerFirstPerson m_cameraPositioner{
      glm::vec3(0.f, 1.f, -1.5f), glm::vec3(0.f, 0.5f, 0.f), glm::vec3(0.f, 1.f, 0.f)};

  eng::Camera m_camera{m_cameraPositioner};

  eng::ImGuiLayer m_imgui;
};