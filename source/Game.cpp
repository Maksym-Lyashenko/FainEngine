#include "Game.h"

#include <array>
#include <cstddef>

bool Game::Init()
{
  eng::IContext& ctx = eng::Engine::GetInstance().GetVulkanContext();

  const std::array<Vertex, 3> vertices = {
      Vertex{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
      Vertex{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
      Vertex{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
  };

  m_vertexBuffer.create(
      &ctx.allocator(),
      sizeof(vertices),
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      eng::BufferMemoryUsage::GpuOnly);

  ctx.uploadContext().uploadBuffer(
      &ctx.allocator(), vertices.data(), sizeof(vertices), m_vertexBuffer);

  const eng::ShaderModuleHandle vert = eng::loadShaderModule(&ctx, "source/main.vert.spv");
  const eng::ShaderModuleHandle frag = eng::loadShaderModule(&ctx, "source/main.frag.spv");

  const std::array<VkVertexInputBindingDescription, 1> bindings = {VkVertexInputBindingDescription{
      .binding = 0,
      .stride = sizeof(Vertex),
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
  }};

  const std::array<VkVertexInputAttributeDescription, 2> attributes = {
      VkVertexInputAttributeDescription{
          .location = 0,
          .binding = 0,
          .format = VK_FORMAT_R32G32_SFLOAT,
          .offset = static_cast<uint32_t>(offsetof(Vertex, position)),
      },
      VkVertexInputAttributeDescription{
          .location = 1,
          .binding = 0,
          .format = VK_FORMAT_R32G32B32_SFLOAT,
          .offset = static_cast<uint32_t>(offsetof(Vertex, color)),
      }};

  eng::RenderPipelineDesc rpDesc{};
  rpDesc.smVert = vert;
  rpDesc.smFrag = frag;
  rpDesc.colorFormats[0] = ctx.getSwapchainFormat();
  rpDesc.vertexBindings = bindings;
  rpDesc.vertexAttributes = attributes;
  rpDesc.cullMode = VK_CULL_MODE_NONE;

  m_trianglePipeline = ctx.createRenderPipeline(rpDesc);

  ctx.destroyShaderModule(frag);
  ctx.destroyShaderModule(vert);

  return m_trianglePipeline != eng::kInvalidHandle;
}

void Game::Update(float DeltaTime)
{
  (void)DeltaTime;

  if (m_trianglePipeline == eng::kInvalidHandle || !m_vertexBuffer.isValid())
  {
    return;
  }

  eng::IContext& ctx = eng::Engine::GetInstance().GetVulkanContext();

  eng::ICommandBuffer& cmd = ctx.acquireCommandBuffer();

  eng::BeginRenderingDesc beginDesc{};
  beginDesc.color[0].loadOp = eng::LoadOp::Clear;
  beginDesc.color[0].clearColor = {1.0f, 1.0f, 1.0f, 1.0f};

  eng::RenderingTargets targets{};
  targets.color[0] = ctx.getCurrentSwapchainTexture();

  cmd.cmdBeginRendering(beginDesc, targets);
  cmd.cmdBindRenderPipeline(m_trianglePipeline);
  cmd.cmdBindVertexBuffer(m_vertexBuffer.handle());
  cmd.cmdPushDebugGroupLabel("Render Triangle", 0xff0000ff);
  cmd.cmdDraw(3);
  cmd.cmdPopDebugGroupLabel();
  cmd.cmdEndRendering();

  ctx.submit(cmd, ctx.getCurrentSwapchainTexture());
}

void Game::Destroy()
{
  eng::IContext& ctx = eng::Engine::GetInstance().GetVulkanContext();

  ctx.waitIdle();

  if (m_trianglePipeline != eng::kInvalidHandle)
  {
    ctx.destroyRenderPipeline(m_trianglePipeline);
    m_trianglePipeline = eng::kInvalidHandle;
  }

  m_vertexBuffer.destroy();
}