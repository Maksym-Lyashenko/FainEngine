#pragma once

#include "vk/VulkanContext.h"
#include "vk/ShaderModule.h"

#include <memory>
#include <chrono>

struct GLFWwindow;

namespace eng
{

class Application;

class Engine
{
 public:
  static Engine& GetInstance();

 private:
  Engine() = default;
  Engine(const Engine&) = delete;
  Engine(Engine&&) = delete;
  Engine& operator=(const Engine&) = delete;
  Engine& operator=(Engine&&) = delete;

 public:
  bool Init(int width, int height, const char* name);
  void Run();
  void Destroy();

  void SetApplication(Application* app);
  Application* GetApplication();
  IContext& GetVulkanContext();
  ShaderModule& GetShaderModule();
  GLFWwindow* GetWindow();

 private:
  std::unique_ptr<Application> m_application;
  std::chrono::steady_clock::time_point m_lastTimePoint;
  GLFWwindow* m_window = nullptr;
  std::unique_ptr<IContext> m_vulkanContext;
  ShaderModule m_shaderModule;
};

}  // namespace eng