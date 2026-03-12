#include "debug/ImGuiLayer.h"

#include "vk/VulkanCommandBuffer.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <cstdint>
#include <type_traits>

namespace eng
{
namespace
{
void vkCheck(VkResult result, const char* what)
{
  if (result != VK_SUCCESS)
  {
    throw std::runtime_error(std::string("Vulkan error at: ") + what);
  }
}

ImTextureID ToImTextureID(VkDescriptorSet set)
{
  static_assert(sizeof(ImTextureID) >= sizeof(uintptr_t));

  if constexpr (std::is_pointer_v<ImTextureID>)
  {
    return reinterpret_cast<ImTextureID>(set);
  }
  else
  {
    return static_cast<ImTextureID>(reinterpret_cast<uintptr_t>(set));
  }
}

VkDescriptorSet FromImTextureID(ImTextureID id)
{
  if constexpr (std::is_pointer_v<ImTextureID>)
  {
    return reinterpret_cast<VkDescriptorSet>(id);
  }
  else
  {
    return reinterpret_cast<VkDescriptorSet>(static_cast<uintptr_t>(id));
  }
}
}  // namespace

ImGuiLayer::~ImGuiLayer()
{
  destroy();
}

void ImGuiLayer::create(IContext& ctx, GLFWwindow* window)
{
  if (m_ctx != nullptr)
  {
    throw std::runtime_error("ImGuiLayer::create(): already created");
  }

  if (window == nullptr)
  {
    throw std::runtime_error("ImGuiLayer::create(): window is null");
  }

  m_ctx = &ctx;
  m_window = window;

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  VkDescriptorPoolSize poolSizes[] = {
      {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
      {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000},
  };

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  poolInfo.maxSets = 1000 * static_cast<uint32_t>(std::size(poolSizes));
  poolInfo.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes));
  poolInfo.pPoolSizes = poolSizes;

  vkCheck(
      vkCreateDescriptorPool(ctx.device(), &poolInfo, nullptr, &m_descriptorPool),
      "vkCreateDescriptorPool (ImGui)");

  ImGui_ImplGlfw_InitForVulkan(window, true);

  ImGui_ImplVulkan_InitInfo initInfo{};
  initInfo.Instance = ctx.instance();
  initInfo.PhysicalDevice = ctx.physicalDevice();
  initInfo.Device = ctx.device();
  initInfo.QueueFamily = ctx.graphicsQueueFamily();
  initInfo.Queue = ctx.graphicsQueue();
  initInfo.DescriptorPool = m_descriptorPool;
  initInfo.MinImageCount = ctx.minImageCount();
  initInfo.ImageCount = ctx.imageCount();
  initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  initInfo.UseDynamicRendering = true;

  VkPipelineRenderingCreateInfo renderingInfo{};
  renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  renderingInfo.colorAttachmentCount = 1;
  VkFormat colorFormat = ctx.getSwapchainFormat();
  renderingInfo.pColorAttachmentFormats = &colorFormat;

  initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = renderingInfo;

  ImGui_ImplVulkan_Init(&initInfo);
}

void ImGuiLayer::destroy()
{
  if (m_ctx == nullptr)
  {
    return;
  }

  m_ctx->waitIdle();

  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  if (m_descriptorPool != VK_NULL_HANDLE)
  {
    vkDestroyDescriptorPool(m_ctx->device(), m_descriptorPool, nullptr);
    m_descriptorPool = VK_NULL_HANDLE;
  }

  m_window = nullptr;
  m_ctx = nullptr;
}

void ImGuiLayer::beginFrame()
{
  if (m_ctx == nullptr)
  {
    throw std::runtime_error("ImGuiLayer::beginFrame(): not initialized");
  }

  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void ImGuiLayer::render(ICommandBuffer& cmd)
{
  if (m_ctx == nullptr)
  {
    throw std::runtime_error("ImGuiLayer::render(): not initialized");
  }

  ImGui::Render();

  VulkanCommandBuffer* vkCmd = dynamic_cast<VulkanCommandBuffer*>(&cmd);
  if (vkCmd == nullptr)
  {
    throw std::runtime_error("ImGuiLayer::render(): ICommandBuffer is not VulkanCommandBuffer");
  }

  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vkCmd->handle());
}

ImTextureID ImGuiLayer::addTexture(const Texture2D& texture)
{
  if (!texture.isValid())
  {
    throw std::runtime_error("ImGuiLayer::addTexture(): invalid texture");
  }

  return ToImTextureID(ImGui_ImplVulkan_AddTexture(
      texture.resource().sampler(),
      texture.resource().imageView(),
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
}

void ImGuiLayer::removeTexture(ImTextureID textureId)
{
  if (textureId != ImTextureID{})
  {
    ImGui_ImplVulkan_RemoveTexture(FromImTextureID(textureId));
  }
}

// call after ImGui::Begin to keep ImGui in GLFW view
void ImGuiLayer::ClampCurrentWindowToMainViewport()
{
  const ImGuiViewport* viewport = ImGui::GetMainViewport();

  const ImVec2 workMin = viewport->WorkPos;
  const ImVec2 workMax = ImVec2(
      viewport->WorkPos.x + viewport->WorkSize.x, viewport->WorkPos.y + viewport->WorkSize.y);

  ImVec2 pos = ImGui::GetWindowPos();
  const ImVec2 size = ImGui::GetWindowSize();

  ImVec2 clampedPos = pos;

  if (clampedPos.x < workMin.x)
  {
    clampedPos.x = workMin.x;
  }
  if (clampedPos.y < workMin.y)
  {
    clampedPos.y = workMin.y;
  }
  if (clampedPos.x + size.x > workMax.x)
  {
    clampedPos.x = workMax.x - size.x;
  }
  if (clampedPos.y + size.y > workMax.y)
  {
    clampedPos.y = workMax.y - size.y;
  }

  if (clampedPos.x != pos.x || clampedPos.y != pos.y)
  {
    ImGui::SetWindowPos(clampedPos);
  }
}

}  // namespace eng