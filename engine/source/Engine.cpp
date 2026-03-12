#include "Engine.h"

#include <GLFW/glfw3.h>

#include "Application.h"
#include "debug/Profiler.h"

#include <iostream>

namespace eng
{

Engine& Engine::GetInstance()
{
  static Engine instance;
  return instance;
}

bool Engine::Init(int width, int height, const char* name)
{
  ENG_PROFILE_ZONE_N("Engine::Init");

  if (!m_application)
  {
    return false;
  }

#if defined(__linux__)
  glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
#endif

  if (!glfwInit())
  {
    return false;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  m_window = glfwCreateWindow(width, height, name, nullptr, nullptr);

  if (m_window == nullptr)
  {
    std::cout << "Error creating window" << std::endl;
    glfwTerminate();
    return false;
  }

  m_vulkanContext = eng::createVulkanContextWithSwapchain(m_window, {});

  try
  {
    return m_application->Init();
  }
  catch (const std::exception& e)
  {
    std::cout << "Application init failed: " << e.what() << '\n';

    m_vulkanContext.reset();

    if (m_window)
    {
      glfwDestroyWindow(m_window);
      m_window = nullptr;
    }

    glfwTerminate();
    return false;
  }
  catch (...)
  {
    std::cout << "Application init failed: unknown exception\n";

    m_vulkanContext.reset();

    if (m_window)
    {
      glfwDestroyWindow(m_window);
      m_window = nullptr;
    }

    glfwTerminate();
    return false;
  }
}

void Engine::Run()
{
  ENG_PROFILE_ZONE_N("Engine::Run");

  if (!m_application)
  {
    return;
  }

  m_lastTimePoint = std::chrono::steady_clock::now();

  while (!glfwWindowShouldClose(m_window) && !m_application->NeedsToBeClosed())
  {
    ENG_PROFILE_ZONE_N("Main Loop");

    glfwPollEvents();

    auto now = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(now - m_lastTimePoint).count();
    m_lastTimePoint = now;

    m_application->Update(deltaTime);
  }
}

void Engine::Destroy()
{
  if (m_application)
  {
    m_application->Destroy();
    m_application.reset();
    glfwTerminate();
    m_window = nullptr;
  }
}

void Engine::SetApplication(Application* app)
{
  m_application.reset(app);
}

Application* Engine::GetApplication()
{
  return m_application.get();
}

IContext& Engine::GetVulkanContext()
{
  return *m_vulkanContext;
}

ShaderModule& Engine::GetShaderModule()
{
  return m_shaderModule;
}

GLFWwindow* Engine::GetWindow()
{
  return m_window;
}

}  // namespace eng