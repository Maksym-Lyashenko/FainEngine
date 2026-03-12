#include "Game.h"

#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <array>

using glm::mat4;
using glm::vec3;

bool Game::Init()
{
  eng::IContext& ctx = eng::Engine::GetInstance().GetVulkanContext();

  const eng::MeshData mesh = eng::LoadFirstMeshPositionsOnly("assets/rubber_duck/scene.gltf");
  if (mesh.vertices.empty() || mesh.indices.empty())
  {
    return false;
  }

  m_mesh.create(ctx, std::span(mesh.vertices), std::span(mesh.indices));

  auto vert = eng::ShaderLibrary::LoadModule(ctx, "source/main.vert.spv");
  auto frag = eng::ShaderLibrary::LoadModule(ctx, "source/main.frag.spv");

  m_pipelineSolid = eng::GraphicsPipelineBuilder(ctx)
                        .Shaders(vert.get(), frag.get())
                        .VertexLayout<eng::VertexP3>()
                        .CullBack()
                        .FillMode(VK_POLYGON_MODE_FILL)
                        .PushConstants(sizeof(glm::mat4), VK_SHADER_STAGE_VERTEX_BIT)
                        .Depth(ctx.getDepthFormat(), true, true, VK_COMPARE_OP_LESS)
                        .Build();

  const uint32_t isWireframe = 1;
  const std::array<VkSpecializationMapEntry, 1> specEntries = {VkSpecializationMapEntry{
      .constantID = 0,
      .offset = 0,
      .size = sizeof(uint32_t),
  }};

  m_pipelineWireframe =
      eng::GraphicsPipelineBuilder(ctx)
          .Shaders(vert.get(), frag.get())
          .VertexLayout<eng::VertexP3>()
          .CullBack()
          .FillMode(VK_POLYGON_MODE_LINE)
          .PushConstants(sizeof(glm::mat4), VK_SHADER_STAGE_VERTEX_BIT)
          .Depth(ctx.getDepthFormat(), true, false, VK_COMPARE_OP_LESS_OR_EQUAL)
          .DepthBias(true)
          .Specialization(
              VK_SHADER_STAGE_FRAGMENT_BIT, specEntries, &isWireframe, sizeof(isWireframe))
          .Build();

  return m_pipelineSolid.valid() && m_pipelineWireframe.valid() && m_mesh.isValid();
}

void Game::Update(float DeltaTime)
{
  (void)DeltaTime;

  if (!m_pipelineSolid.valid() || !m_pipelineWireframe.valid())
  {
    return;
  }

  eng::IContext& ctx = eng::Engine::GetInstance().GetVulkanContext();
  GLFWwindow* window = eng::Engine::GetInstance().GetWindow();
  if (window == nullptr)
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

  const float ratio = static_cast<float>(width) / static_cast<float>(height);

  const mat4 m = glm::rotate(mat4(1.0f), glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
  const mat4 v = glm::rotate(
      glm::translate(mat4(1.0f), vec3(0.0f, -0.5f, -1.5f)),
      static_cast<float>(glfwGetTime()),
      vec3(0.0f, 1.0f, 0.0f));

  mat4 p = glm::perspective(glm::radians(45.0f), ratio, 0.1f, 1000.0f);

  const mat4 mvp = p * v * m;

  eng::ICommandBuffer& cmd = ctx.acquireCommandBuffer();

  eng::BeginRenderingDesc beginDesc{};
  beginDesc.color[0].loadOp = eng::LoadOp::Clear;
  beginDesc.color[0].clearColor = {1.0f, 1.0f, 1.0f, 1.0f};
  beginDesc.useDepth = true;
  beginDesc.depth.loadOp = eng::LoadOp::Clear;
  beginDesc.depth.clearDepth = 1.0f;

  eng::RenderingTargets targets{};
  targets.color[0] = ctx.getCurrentSwapchainTexture();

  cmd.cmdBeginRendering(beginDesc, targets);

  cmd.cmdPushDebugGroupLabel("Mesh", 0xff0000ff);
  {
    m_mesh.bind(cmd);

    cmd.cmdBindRenderPipeline(m_pipelineSolid.get());
    cmd.cmdPushConstants(mvp);
    m_mesh.draw(cmd);

    cmd.cmdBindRenderPipeline(m_pipelineWireframe.get());
    cmd.cmdPushConstants(mvp);
    cmd.cmdSetDepthBias(-1.0f, -1.0f, 0.0f);
    m_mesh.draw(cmd);
    cmd.cmdSetDepthBias(0.0f, 0.0f, 0.0f);
  }
  cmd.cmdPopDebugGroupLabel();

  cmd.cmdEndRendering();
  ctx.submit(cmd, ctx.getCurrentSwapchainTexture());
}

void Game::Destroy()
{
  eng::IContext& ctx = eng::Engine::GetInstance().GetVulkanContext();
  ctx.waitIdle();

  m_pipelineWireframe.reset();
  m_pipelineSolid.reset();

  m_mesh.destroy();
}