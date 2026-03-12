#pragma once

#include "vk/VulkanContext.h"

#include "graphics/Texture2D.h"

#include <imgui.h>
#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace eng
{
class ImGuiLayer
{
 public:
  ImGuiLayer() = default;
  ~ImGuiLayer();

  ImGuiLayer(const ImGuiLayer&) = delete;
  ImGuiLayer& operator=(const ImGuiLayer&) = delete;

  void create(IContext& ctx, GLFWwindow* window);
  void destroy();

  void beginFrame();
  void render(ICommandBuffer& cmd);

  ImTextureID addTexture(const Texture2D& texture);
  void removeTexture(ImTextureID textureId);

  bool isValid() const { return m_ctx != nullptr; }

  // call after ImGui::Begin to keep ImGui in GLFW view
  static void ClampCurrentWindowToMainViewport();

 private:
  IContext* m_ctx = nullptr;
  GLFWwindow* m_window = nullptr;
  VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
};
}  // namespace eng