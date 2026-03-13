#include "Game.h"

#include "debug/ProfilerVulkan.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <implot.h>

using glm::mat4;
using glm::vec3;

bool Game::Init()
{
  m_ctx = &eng::Engine::GetInstance().GetVulkanContext();
  m_window = eng::Engine::GetInstance().GetWindow();
  if (m_ctx == nullptr || m_window == nullptr)
  {
    return false;
  }

  m_imgui.create(*m_ctx, m_window, "assets/OpenSans-Light.ttf", 30.f);

  return m_imgui.isValid();
}

void Game::Update(float DeltaTime)
{
  // (void)DeltaTime;

  if (m_ctx == nullptr || m_window == nullptr)
  {
    return;
  }

  m_fpsCounter.tick(DeltaTime);

  int width = 0;
  int height = 0;
  glfwGetFramebufferSize(m_window, &width, &height);
  if (width == 0 || height == 0)
  {
    return;
  }

  eng::ICommandBuffer& cmd = m_ctx->acquireCommandBuffer();

  eng::BeginRenderingDesc beginDesc{};
  beginDesc.color[0].loadOp = eng::LoadOp::Clear;
  beginDesc.color[0].clearColor = {1.f, 1.f, 1.f, 1.f};

  eng::RenderingTargets targets{};
  targets.color[0] = m_ctx->getCurrentSwapchainTexture();

  cmd.cmdBeginRendering(beginDesc, targets);

  m_imgui.beginFrame();

  if (const ImGuiViewport* v = ImGui::GetMainViewport())
  {
    ImGui::SetNextWindowPos(
        {v->WorkPos.x + v->WorkSize.x - 15.f, v->WorkPos.y + 15.f}, ImGuiCond_Always, {1.f, 0.f});
  }

  ImGui::SetNextWindowBgAlpha(.3f);
  ImGui::SetNextWindowSize(ImVec2(ImGui::CalcTextSize("FPS:________").x, 0.f));

  if (ImGui::Begin(
          "##FPS",
          nullptr,
          ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
              ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
              ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove))
  {
    ImGui::Text("FPS : %i", static_cast<int>(m_fpsCounter.getFPS()));
    ImGui::Text("Ms  : %.1f", m_fpsCounter.getMs());
  }
  ImGui::End();

  ImPlot::ShowDemoWindow();
  ImGui::ShowDemoWindow();

  m_imgui.render(cmd);

  cmd.cmdEndRendering();
  m_ctx->submit(cmd, m_ctx->getCurrentSwapchainTexture());
}

void Game::Destroy()
{
  if (m_ctx != nullptr)
  {
    m_ctx->waitIdle();
  }
  m_imgui.destroy();
  m_ctx = nullptr;
  m_window = nullptr;
}