#include "Game.h"

#include "debug/ProfilerVulkan.h"

#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <stb_image.h>

#include <utils/Bitmap.h>
#include <utils/UtilsCubemap.h>

#include <iostream>
#include <span>
#include <stdexcept>

bool Game::Init()
{
  try
  {
    m_ctx = &eng::Engine::GetInstance().GetVulkanContext();
    m_window = eng::Engine::GetInstance().GetWindow();

    if (m_ctx == nullptr || m_window == nullptr)
    {
      return false;
    }

    glfwSetWindowUserPointer(m_window, this);

    glfwSetCursorPosCallback(
        m_window,
        [](GLFWwindow* window, double x, double y)
        {
          auto* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
          if (game == nullptr)
          {
            return;
          }

          int width = 0;
          int height = 0;
          glfwGetFramebufferSize(window, &width, &height);
          if (width == 0 || height == 0)
          {
            return;
          }

          game->m_mouseState.pos.x = static_cast<float>(x / static_cast<double>(width));
          game->m_mouseState.pos.y = 1.0f - static_cast<float>(y / static_cast<double>(height));
        });

    glfwSetMouseButtonCallback(
        m_window,
        [](GLFWwindow* window, int button, int action, int /*mods*/)
        {
          auto* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
          if (game == nullptr)
          {
            return;
          }

          if (button == GLFW_MOUSE_BUTTON_LEFT)
          {
            game->m_mouseState.pressedLeft = (action == GLFW_PRESS);
          }
        });

    glfwSetKeyCallback(
        m_window,
        [](GLFWwindow* window, int key, int /*scancode*/, int action, int mods)
        {
          auto* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
          if (game == nullptr)
          {
            return;
          }

          const bool pressed = (action != GLFW_RELEASE);

          if (key == GLFW_KEY_ESCAPE && pressed)
          {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
          }

          if (key == GLFW_KEY_W)
          {
            game->m_cameraPositioner.movement.forward = pressed;
          }
          if (key == GLFW_KEY_S)
          {
            game->m_cameraPositioner.movement.backward = pressed;
          }
          if (key == GLFW_KEY_A)
          {
            game->m_cameraPositioner.movement.left = pressed;
          }
          if (key == GLFW_KEY_D)
          {
            game->m_cameraPositioner.movement.right = pressed;
          }
          if (key == GLFW_KEY_1)
          {
            game->m_cameraPositioner.movement.up = pressed;
          }
          if (key == GLFW_KEY_2)
          {
            game->m_cameraPositioner.movement.down = pressed;
          }

          game->m_cameraPositioner.movement.fastSpeed = ((mods & GLFW_MOD_SHIFT) != 0) && pressed;

          if (key == GLFW_KEY_SPACE && pressed)
          {
            game->m_cameraPositioner.lookAt(
                glm::vec3(0.0f, 1.0f, -1.5f),
                glm::vec3(0.0f, 0.5f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f));

            game->m_cameraPositioner.setSpeed(glm::vec3(0.0f));
          }
        });

    auto vertMain = eng::ShaderLibrary::LoadModule(*m_ctx, "source/main.vert.spv");
    auto fragMain = eng::ShaderLibrary::LoadModule(*m_ctx, "source/main.frag.spv");

    auto vertSkybox = eng::ShaderLibrary::LoadModule(*m_ctx, "source/skybox.vert.spv");
    auto fragSkybox = eng::ShaderLibrary::LoadModule(*m_ctx, "source/skybox.frag.spv");

    {
      int w = 0;
      int h = 0;
      int comp = 0;

      float* hdrPixels = stbi_loadf("assets/piazza_bologni_1k.hdr", &w, &h, &comp, 4);
      if (hdrPixels == nullptr || w <= 0 || h <= 0)
      {
        throw std::runtime_error("Failed to load HDR cubemap source: assets/piazza_bologni_1k.hdr");
      }

      Bitmap in(w, h, 4, eBitmapFormat_Float, hdrPixels);
      Bitmap verticalCross = convertEquirectangularMapToVerticalCross(in);
      stbi_image_free(hdrPixels);

      Bitmap cubemap = convertVerticalCrossToCubeMapFaces(verticalCross);

      m_environmentCube.createFromRGBA32F(
          *m_ctx,
          static_cast<uint32_t>(cubemap.w_),
          static_cast<uint32_t>(cubemap.h_),
          cubemap.data_.data(),
          cubemap.data_.size());
    }

    m_sceneUniforms.create(*m_ctx);

    m_skyboxPass.create(
        *m_ctx, vertSkybox.get(), fragSkybox.get(), m_sceneUniforms, m_environmentCube);

    const eng::MeshDataP3N3UV2 meshData = eng::LoadFirstMeshP3N3UV2("assets/drone/drone.gltf");

    m_duck.create(
        *m_ctx,
        vertMain.get(),
        fragMain.get(),
        m_sceneUniforms,
        m_environmentCube,
        std::span(meshData.vertices.data(), meshData.vertices.size()),
        std::span(meshData.indices.data(), meshData.indices.size()),
        "assets/rubber_duck/textures/Duck_baseColor.png"

    );

    m_imgui.create(*m_ctx, m_window, "assets/OpenSans-Light.ttf", 30.f);

    return m_environmentCube.isValid() && m_sceneUniforms.isValid() && m_skyboxPass.isValid() &&
           m_duck.isValid();
  }
  catch (const std::exception& e)
  {
    std::cerr << "Game::Init failed: " << e.what() << '\n';
    return false;
  }
}

void Game::Update(float DeltaTime)
{
  (void)DeltaTime;

  if (m_ctx == nullptr || m_window == nullptr)
  {
    return;
  }

  if (!m_environmentCube.isValid() || !m_sceneUniforms.isValid() || !m_skyboxPass.isValid() ||
      !m_duck.isValid())
  {
    return;
  }

  int width = 0;
  int height = 0;
  glfwGetFramebufferSize(m_window, &width, &height);
  if (width == 0 || height == 0)
  {
    return;
  }

  m_cameraPositioner.update(DeltaTime, m_mouseState.pos, m_mouseState.pressedLeft);

  const float ratio = static_cast<float>(width) / static_cast<float>(height);
  const glm::vec3 cameraPos = m_camera.getPosition();

  glm::mat4 proj = glm::perspective(glm::radians(60.0f), ratio, 0.1f, 1000.0f);

  const glm::mat4 tilt =
      glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

  const glm::mat4 spin =
      glm::rotate(glm::mat4(1.0f), static_cast<float>(glfwGetTime()), glm::vec3(0.0f, 1.0f, 0.0f));

  const glm::mat4 model = spin * tilt;

  const glm::mat4 view = m_camera.getViewMatrix();

  const eng::PerFrameData perFrame{
      .model = model,
      .view = view,
      .proj = proj,
      .cameraPos = glm::vec4(cameraPos, 1.0f),
  };

  m_sceneUniforms.update(perFrame);

  eng::ICommandBuffer& cmd = m_ctx->acquireCommandBuffer();

  auto* gpuCtx = static_cast<tracy::VkCtx*>(m_ctx->gpuProfilerContext());

  eng::BeginRenderingDesc beginDesc{};
  beginDesc.color[0].loadOp = eng::LoadOp::Clear;
  beginDesc.color[0].clearColor = {1.0f, 1.0f, 1.0f, 1.0f};
  beginDesc.useDepth = true;
  beginDesc.depth.loadOp = eng::LoadOp::Clear;
  beginDesc.depth.clearDepth = 1.0f;

  eng::RenderingTargets targets{};
  targets.color[0] = m_ctx->getCurrentSwapchainTexture();

  cmd.cmdBeginRendering(beginDesc, targets);

  cmd.cmdPushDebugGroupLabel("Skybox", 0xff0000ff);
  {
    ENG_PROFILE_VK_ZONE(gpuCtx, cmd.handle(), "Skybox");
    m_skyboxPass.draw(cmd);
  }
  cmd.cmdPopDebugGroupLabel();

  cmd.cmdPushDebugGroupLabel("Duck Solid", 0xff0000ff);
  {
    ENG_PROFILE_VK_ZONE(gpuCtx, cmd.handle(), "Mesh Solid");
    m_duck.drawSolid(cmd);
  }
  cmd.cmdPopDebugGroupLabel();

  // cmd.cmdPushDebugGroupLabel("Mesh Wireframe", 0xff0000ff);
  // {
  //   ENG_PROFILE_VK_ZONE(gpuCtx, cmd.handle(), "Mesh Wireframe");
  //   m_duck.drawWireframe(cmd);
  // }
  // cmd.cmdPopDebugGroupLabel();

  cmd.cmdEndRendering();

  eng::BeginRenderingDesc uiBeginDesc{};
  uiBeginDesc.color[0].loadOp = eng::LoadOp::Load;

  eng::RenderingTargets uiTargets{};
  uiTargets.color[0] = m_ctx->getCurrentSwapchainTexture();

  cmd.cmdBeginRendering(uiBeginDesc, uiTargets);

  m_imgui.beginFrame();

  ImGui::SetNextWindowPos(ImVec2(10, 10));
  ImGui::Begin(
      "Keyboard hints:", nullptr,
      ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing |
          ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoCollapse);
  ImGui::Text("W/S/A/D - camera movement");
  ImGui::Text("1/2 - camera up/down");
  ImGui::Text("Shift - fast movement");
  ImGui::Text("Space - reset view");
  ImGui::End();

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

  m_duck.destroy();
  m_skyboxPass.destroy();
  m_sceneUniforms.destroy();
  m_environmentCube.destroy();
  m_imgui.destroy();

  m_ctx = nullptr;
  m_window = nullptr;
}