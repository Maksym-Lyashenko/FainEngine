#include "Game.h"

#include "eng.h"

#include <GLFW/glfw3.h>
#include <imgui.h>

bool Game::Init()
{
  eng::IContext& ctx = eng::Engine::GetInstance().GetVulkanContext();
  GLFWwindow* window = eng::Engine::GetInstance().GetWindow();
  if (window == nullptr)
  {
    return false;
  }

  m_imgui.create(ctx, window);
  m_texture.createFromFile(ctx, "assets/wood.jpg");
  m_imguiTexture = m_imgui.addTexture(m_texture);

  return m_texture.isValid();
}

void Game::Update(float DeltaTime)
{
  (void)DeltaTime;

  eng::IContext& ctx = eng::Engine::GetInstance().GetVulkanContext();

  eng::ICommandBuffer& cmd = ctx.acquireCommandBuffer();

  eng::BeginRenderingDesc beginDesc{};
  beginDesc.color[0].loadOp = eng::LoadOp::Clear;
  beginDesc.color[0].clearColor = {1.0f, 1.0f, 1.0f, 1.0f};

  eng::RenderingTargets targets{};
  targets.color[0] = ctx.getCurrentSwapchainTexture();

  cmd.cmdBeginRendering(beginDesc, targets);

  m_imgui.beginFrame();

  ImGui::Begin("Texture Viewer", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
  ImGui::Image(m_imguiTexture, ImVec2(512.0f, 512.0f));
  eng::ImGuiLayer::ClampCurrentWindowToMainViewport();
  ImGui::End();

  m_imgui.render(cmd);

  cmd.cmdEndRendering();
  ctx.submit(cmd, ctx.getCurrentSwapchainTexture());
}

void Game::Destroy()
{
  eng::IContext& ctx = eng::Engine::GetInstance().GetVulkanContext();
  ctx.waitIdle();

  if (m_imguiTexture != ImTextureID{})
  {
    m_imgui.removeTexture(m_imguiTexture);
    m_imguiTexture = ImTextureID{};
  }

  m_texture.destroy();
  m_imgui.destroy();
}