#include "Engine.h"

#include <GLFW/glfw3.h>

#include "Application.h"

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

  glfwMakeContextCurrent(m_window);

  return m_application->Init();
}

void Engine::Run()
{
  if (!m_application)
  {
    return;
  }

  m_lastTimePoint = std::chrono::steady_clock::now();

  while (!glfwWindowShouldClose(m_window) && !m_application->NeedsToBeClosed())
  {
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

}  // namespace eng