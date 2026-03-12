#include "vk/VulkanContext.h"

#include "vk/VulkanCommandBuffer.h"
#include "vk/VulkanSwapchain.h"
#include "vk/VulkanAllocator.h"
#include "vk/VulkanUploadContext.h"
#include "debug/Profiler.h"
#include "debug/ProfilerVulkan.h"

#include <GLFW/glfw3.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

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

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void*)
{
  const char* sev = (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)     ? "ERROR"
                    : (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) ? "WARN"
                    : (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)    ? "INFO"
                                                                                   : "VERBOSE";

  std::cerr << "[VK " << sev << "] " << callbackData->pMessage << '\n';
  return VK_FALSE;
}

VkDebugUtilsMessengerEXT createDebugMessenger(VkInstance instance)
{
  auto fn = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

  if (fn == nullptr)
  {
    return VK_NULL_HANDLE;
  }

  VkDebugUtilsMessengerCreateInfoEXT ci{};
  ci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  ci.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  ci.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  ci.pfnUserCallback = debugCallback;

  VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
  vkCheck(fn(instance, &ci, nullptr, &messenger), "vkCreateDebugUtilsMessengerEXT");

  return messenger;
}

void destroyDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger)
{
  if (messenger == VK_NULL_HANDLE)
  {
    return;
  }

  auto fn = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

  if (fn != nullptr)
  {
    fn(instance, messenger, nullptr);
  }
}

bool hasValidationLayer()
{
  uint32_t count = 0;
  vkEnumerateInstanceLayerProperties(&count, nullptr);

  std::vector<VkLayerProperties> layers(count);
  if (count > 0)
  {
    vkEnumerateInstanceLayerProperties(&count, layers.data());
  }

  for (const VkLayerProperties& layer : layers)
  {
    if (std::strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0)
    {
      return true;
    }
  }

  return false;
}

std::vector<const char*> getRequiredInstanceExtensions(bool enableValidation)
{
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

  if (enableValidation)
  {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}

struct QueueFamilyIndices
{
  std::optional<uint32_t> graphics;
  std::optional<uint32_t> present;

  bool complete() const { return graphics.has_value() && present.has_value(); }
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice gpu, VkSurfaceKHR surface)
{
  QueueFamilyIndices indices{};

  uint32_t count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, nullptr);

  std::vector<VkQueueFamilyProperties> families(count);
  if (count > 0)
  {
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, families.data());
  }

  for (uint32_t i = 0; i < count; ++i)
  {
    if ((families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
    {
      indices.graphics = i;
    }

    VkBool32 presentSupport = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &presentSupport);
    if (presentSupport == VK_TRUE)
    {
      indices.present = i;
    }

    if (indices.complete())
    {
      break;
    }
  }

  return indices;
}

bool hasRequiredDeviceExtensions(VkPhysicalDevice gpu)
{
  uint32_t extensionCount = 0;
  vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> extensions(extensionCount);
  if (extensionCount > 0)
  {
    vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, extensions.data());
  }

  bool hasSwapchain = false;
  for (const VkExtensionProperties& ext : extensions)
  {
    if (std::strcmp(ext.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
    {
      hasSwapchain = true;
      break;
    }
  }

  return hasSwapchain;
}

}  // namespace

class VulkanContext final : public IContext, private IVulkanPipelineProvider
{
 public:
  VulkanContext(GLFWwindow* window, const ContextCreateInfo& ci) : m_window(window), m_ci(ci)
  {
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createDevice();

    m_allocator.create(m_instance, m_gpu, m_device, VK_API_VERSION_1_3);

    m_pipelineCache.create(m_device);

    m_uploadContext.create(m_device, m_graphicsFamily, m_graphicsQueue);

    createTracyGpuContext();

    m_swapchain.create(
        m_gpu,
        m_device,
        &m_allocator,
        m_surface,
        m_window,
        m_graphicsFamily,
        m_presentFamily,
        m_ci.preferredPresentMode,
        m_ci.preferredColorFormat);

    createFrames();
    loadDebugUtilsDeviceFns();

    m_cmd.setup(
        m_device, &m_swapchain, this, m_vkCmdBeginDebugUtilsLabelEXT, m_vkCmdEndDebugUtilsLabelEXT);
  }

  ~VulkanContext() override
  {
    if (m_device != VK_NULL_HANDLE)
    {
      vkDeviceWaitIdle(m_device);
    }

    destroyFrames();
    m_swapchain.destroy();
    m_pipelineCache.destroy();

    destroyTracyGpuContext();

    m_uploadContext.destroy();
    m_allocator.destroy();

    if (m_device != VK_NULL_HANDLE)
    {
      vkDestroyDevice(m_device, nullptr);
      m_device = VK_NULL_HANDLE;
    }

    if (m_surface != VK_NULL_HANDLE)
    {
      vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
      m_surface = VK_NULL_HANDLE;
    }

    destroyDebugMessenger(m_instance, m_debugMessenger);

    if (m_instance != VK_NULL_HANDLE)
    {
      vkDestroyInstance(m_instance, nullptr);
      m_instance = VK_NULL_HANDLE;
    }
  }

  VkInstance instance() const override { return m_instance; }
  VkPhysicalDevice physicalDevice() const override { return m_gpu; }
  VkDevice device() const override { return m_device; }
  VkQueue graphicsQueue() const override { return m_graphicsQueue; }
  uint32_t graphicsQueueFamily() const override { return m_graphicsFamily; }
  uint32_t minImageCount() const override { return m_ci.framesInFlight; }
  uint32_t imageCount() const override { return m_swapchain.imageCount(); }

  ICommandBuffer& acquireCommandBuffer() override
  {
    ENG_PROFILE_ZONE_N("VulkanContext acquireCommandBuffer()");

    if (m_frameActive)
    {
      return m_cmd;
    }

    beginFrame();
    return m_cmd;
  }

  void submit(ICommandBuffer& cmd, TextureHandle presentTexture) override
  {
    ENG_PROFILE_ZONE_N("VulkanContext submit");

    if (&cmd != &m_cmd)
    {
      throw std::runtime_error("submit(): foreign command buffer");
    }

    if (!m_frameActive)
    {
      throw std::runtime_error("submit(): no active frame");
    }

    FrameData& frame = m_frames[m_frameIndex];

    m_cmd.finalizeIfNeeded();

#if defined(ENG_ENABLE_TRACY)
    if (m_tracyVkCtx != nullptr)
    {
      TracyVkCollect(m_tracyVkCtx, frame.cmd);
    }
#endif

    vkCheck(vkEndCommandBuffer(frame.cmd), "vkEndCommandBuffer");

    const VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    const VkSemaphore renderComplete = m_swapchain.renderCompleteSemaphore(presentTexture.id);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &frame.imageAvailable;
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &frame.cmd;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderComplete;

    vkCheck(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, frame.inFlight), "vkQueueSubmit");

    const VkSwapchainKHR swapchain = m_swapchain.handle();
    const uint32_t imageIndex = presentTexture.id;

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderComplete;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &imageIndex;

    const VkResult presentResult = vkQueuePresentKHR(m_presentQueue, &presentInfo);

    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR)
    {
      m_swapchain.recreate(m_window);
    }
    else
    {
      vkCheck(presentResult, "vkQueuePresentKHR");
    }

    m_frameActive = false;
    m_frameIndex = (m_frameIndex + 1) % static_cast<uint32_t>(m_frames.size());
  }

  void waitIdle() override
  {
    if (m_device != VK_NULL_HANDLE)
    {
      vkDeviceWaitIdle(m_device);
    }
  }

  void* gpuProfilerContext() const override { return m_tracyVkCtx; }

  ShaderModuleHandle createShaderModule(const ShaderModuleDesc& desc) override
  {
    return m_pipelineCache.createShaderModule(desc);
  }

  void destroyShaderModule(ShaderModuleHandle handle) override
  {
    m_pipelineCache.destroyShaderModule(handle);
  }

  RenderPipelineHandle createRenderPipeline(const RenderPipelineDesc& desc) override
  {
    return m_pipelineCache.createRenderPipeline(desc);
  }

  void destroyRenderPipeline(RenderPipelineHandle handle) override
  {
    m_pipelineCache.destroyRenderPipeline(handle);
  }

  VulkanAllocator& allocator() override { return m_allocator; }

  VulkanUploadContext& uploadContext() override { return m_uploadContext; }

  VkFormat getSwapchainFormat() const override { return m_swapchain.format(); }

  VkExtent2D getSwapchainExtent() const override { return m_swapchain.extent(); }

  TextureHandle getCurrentSwapchainTexture() const override
  {
    return TextureHandle{m_currentImageIndex};
  }

  VkFormat getDepthFormat() const override { return m_swapchain.depthFormat(); }

  VkPipelineLayout getPipelineLayout(RenderPipelineHandle handle) const override
  {
    return m_pipelineCache.getPipelineLayout(handle);
  }

 private:
  VkPipeline getPipeline(RenderPipelineHandle handle) const override
  {
    return m_pipelineCache.getPipeline(handle);
  }

 private:
  struct FrameData
  {
    VkCommandPool cmdPool = VK_NULL_HANDLE;
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    VkSemaphore imageAvailable = VK_NULL_HANDLE;
    VkFence inFlight = VK_NULL_HANDLE;
  };

 private:
  void createInstance()
  {
    const bool enableValidation = m_ci.enableValidation && hasValidationLayer();

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "FainEngine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "FainEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    const std::vector<const char*> extensions = getRequiredInstanceExtensions(enableValidation);

    VkInstanceCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pApplicationInfo = &appInfo;
    ci.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    ci.ppEnabledExtensionNames = extensions.data();

    const char* validationLayer = "VK_LAYER_KHRONOS_validation";
    if (enableValidation)
    {
      ci.enabledLayerCount = 1;
      ci.ppEnabledLayerNames = &validationLayer;
    }

    vkCheck(vkCreateInstance(&ci, nullptr, &m_instance), "vkCreateInstance");

    if (enableValidation)
    {
      m_debugMessenger = createDebugMessenger(m_instance);
    }
  }

  void createSurface()
  {
    vkCheck(
        static_cast<VkResult>(glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface)),
        "glfwCreateWindowSurface");
  }

  void pickPhysicalDevice()
  {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(m_instance, &count, nullptr);

    if (count == 0)
    {
      throw std::runtime_error("No Vulkan physical devices found");
    }

    std::vector<VkPhysicalDevice> gpus(count);
    vkEnumeratePhysicalDevices(m_instance, &count, gpus.data());

    for (VkPhysicalDevice gpu : gpus)
    {
      const QueueFamilyIndices indices = findQueueFamilies(gpu, m_surface);
      if (!indices.complete())
      {
        continue;
      }

      if (!hasRequiredDeviceExtensions(gpu))
      {
        continue;
      }

      m_gpu = gpu;
      m_graphicsFamily = *indices.graphics;
      m_presentFamily = *indices.present;
      return;
    }

    throw std::runtime_error("No suitable Vulkan GPU found");
  }

  void createDevice()
  {
    std::set<uint32_t> uniqueQueueFamilies = {m_graphicsFamily, m_presentFamily};

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float priority = 1.0f;

    for (uint32_t family : uniqueQueueFamilies)
    {
      VkDeviceQueueCreateInfo qci{};
      qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      qci.queueFamilyIndex = family;
      qci.queueCount = 1;
      qci.pQueuePriorities = &priority;
      queueCreateInfos.push_back(qci);
    }

    VkPhysicalDeviceFeatures supportedFeatures{};
    vkGetPhysicalDeviceFeatures(m_gpu, &supportedFeatures);

    if (supportedFeatures.fillModeNonSolid != VK_TRUE)
    {
      throw std::runtime_error("Selected GPU does not support fillModeNonSolid");
    }

    VkPhysicalDeviceFeatures enabledFeatures{};
    enabledFeatures.fillModeNonSolid = VK_TRUE;

    VkPhysicalDeviceVulkan13Features vulkan13Features{};
    vulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    vulkan13Features.dynamicRendering = VK_TRUE;

    const char* deviceExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    VkDeviceCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    ci.pNext = &vulkan13Features;
    ci.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    ci.pQueueCreateInfos = queueCreateInfos.data();
    ci.enabledExtensionCount = static_cast<uint32_t>(std::size(deviceExtensions));
    ci.ppEnabledExtensionNames = deviceExtensions;
    ci.pEnabledFeatures = &enabledFeatures;

    vkCheck(vkCreateDevice(m_gpu, &ci, nullptr, &m_device), "vkCreateDevice");

    vkGetDeviceQueue(m_device, m_graphicsFamily, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_presentFamily, 0, &m_presentQueue);
  }

  void createFrames()
  {
    m_frames.resize(m_ci.framesInFlight);

    for (FrameData& frame : m_frames)
    {
      VkCommandPoolCreateInfo poolCi{};
      poolCi.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
      poolCi.queueFamilyIndex = m_graphicsFamily;
      poolCi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
      vkCheck(
          vkCreateCommandPool(m_device, &poolCi, nullptr, &frame.cmdPool), "vkCreateCommandPool");

      VkCommandBufferAllocateInfo alloc{};
      alloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      alloc.commandPool = frame.cmdPool;
      alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      alloc.commandBufferCount = 1;
      vkCheck(vkAllocateCommandBuffers(m_device, &alloc, &frame.cmd), "vkAllocateCommandBuffers");

      VkSemaphoreCreateInfo semCi{};
      semCi.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
      vkCheck(
          vkCreateSemaphore(m_device, &semCi, nullptr, &frame.imageAvailable), "vkCreateSemaphore");

      VkFenceCreateInfo fenceCi{};
      fenceCi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
      fenceCi.flags = VK_FENCE_CREATE_SIGNALED_BIT;
      vkCheck(vkCreateFence(m_device, &fenceCi, nullptr, &frame.inFlight), "vkCreateFence");
    }
  }

  void destroyFrames()
  {
    for (FrameData& frame : m_frames)
    {
      if (frame.inFlight != VK_NULL_HANDLE)
      {
        vkDestroyFence(m_device, frame.inFlight, nullptr);
        frame.inFlight = VK_NULL_HANDLE;
      }

      if (frame.imageAvailable != VK_NULL_HANDLE)
      {
        vkDestroySemaphore(m_device, frame.imageAvailable, nullptr);
        frame.imageAvailable = VK_NULL_HANDLE;
      }

      if (frame.cmdPool != VK_NULL_HANDLE)
      {
        vkDestroyCommandPool(m_device, frame.cmdPool, nullptr);
        frame.cmdPool = VK_NULL_HANDLE;
      }

      frame.cmd = VK_NULL_HANDLE;
    }

    m_frames.clear();
  }

  void beginFrame()
  {
    ENG_PROFILE_ZONE_N("VulkanContext begineFrame");

    FrameData& frame = m_frames[m_frameIndex];

    vkCheck(vkWaitForFences(m_device, 1, &frame.inFlight, VK_TRUE, UINT64_MAX), "vkWaitForFences");

    const VkResult acquireResult = vkAcquireNextImageKHR(
        m_device,
        m_swapchain.handle(),
        UINT64_MAX,
        frame.imageAvailable,
        VK_NULL_HANDLE,
        &m_currentImageIndex);

    if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
      m_swapchain.recreate(m_window);
      beginFrame();
      return;
    }

    if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR)
    {
      vkCheck(acquireResult, "vkAcquireNextImageKHR");
    }

    vkCheck(vkResetFences(m_device, 1, &frame.inFlight), "vkResetFences");
    vkCheck(vkResetCommandPool(m_device, frame.cmdPool, 0), "vkResetCommandPool");

    VkCommandBufferBeginInfo begin{};
    begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkCheck(vkBeginCommandBuffer(frame.cmd, &begin), "vkBeginCommandBuffer");

    m_cmd.begin(frame.cmd);
    m_frameActive = true;
  }

  void loadDebugUtilsDeviceFns()
  {
    m_vkCmdBeginDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(
        vkGetDeviceProcAddr(m_device, "vkCmdBeginDebugUtilsLabelEXT"));

    m_vkCmdEndDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(
        vkGetDeviceProcAddr(m_device, "vkCmdEndDebugUtilsLabelEXT"));
  }

  void createTracyGpuContext()
  {
#if defined(ENG_ENABLE_TRACY)
    const VkCommandBuffer tracyCmd = m_uploadContext.commandBuffer();

    if (tracyCmd == VK_NULL_HANDLE)
    {
      throw std::runtime_error("createTracyGpuContext(): invalid upload command buffer");
    }

#if defined(TRACY_VK_USE_SYMBOL_TABLE)
    m_tracyVkCtx = TracyVkContextCalibrated(
        m_instance,
        m_gpu,
        m_device,
        m_graphicsQueue,
        tracyCmd,
        vkGetInstanceProcAddr,
        vkGetDeviceProcAddr);
#else
    auto gpdctd = reinterpret_cast<PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT>(
        vkGetInstanceProcAddr(m_instance, "vkGetPhysicalDeviceCalibrateableTimeDomainsEXT"));

    auto gct = reinterpret_cast<PFN_vkGetCalibratedTimestampsEXT>(
        vkGetDeviceProcAddr(m_device, "vkGetCalibratedTimestampsEXT"));

    m_tracyVkCtx =
        TracyVkContextCalibrated(m_gpu, m_device, m_graphicsQueue, tracyCmd, gpdctd, gct);
#endif

    TracyVkContextName(m_tracyVkCtx, "Graphics Queue", 14);
#endif
  }

  void destroyTracyGpuContext()
  {
#if defined(ENG_ENABLE_TRACY)
    if (m_tracyVkCtx != nullptr)
    {
      TracyVkDestroy(m_tracyVkCtx);
      m_tracyVkCtx = nullptr;
    }
#endif
  }

 private:
  GLFWwindow* m_window = nullptr;
  ContextCreateInfo m_ci{};

  VkInstance m_instance = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
  VkSurfaceKHR m_surface = VK_NULL_HANDLE;

  VkPhysicalDevice m_gpu = VK_NULL_HANDLE;
  VkDevice m_device = VK_NULL_HANDLE;

  uint32_t m_graphicsFamily = 0;
  uint32_t m_presentFamily = 0;

  VkQueue m_graphicsQueue = VK_NULL_HANDLE;
  VkQueue m_presentQueue = VK_NULL_HANDLE;

  std::vector<FrameData> m_frames;
  uint32_t m_frameIndex = 0;
  uint32_t m_currentImageIndex = 0;
  bool m_frameActive = false;

  VulkanSwapchain m_swapchain;
  VulkanPipelineCache m_pipelineCache;
  VulkanAllocator m_allocator;
  VulkanUploadContext m_uploadContext;
  VulkanCommandBuffer m_cmd;

  PFN_vkCmdBeginDebugUtilsLabelEXT m_vkCmdBeginDebugUtilsLabelEXT = nullptr;
  PFN_vkCmdEndDebugUtilsLabelEXT m_vkCmdEndDebugUtilsLabelEXT = nullptr;

#if defined(ENG_ENABLE_TRACY)
  tracy::VkCtx* m_tracyVkCtx = nullptr;
#endif
};

std::unique_ptr<IContext> createVulkanContextWithSwapchain(
    GLFWwindow* window, const ContextCreateInfo& ci)
{
  return std::make_unique<VulkanContext>(window, ci);
}

std::vector<uint32_t> readSpirvFile(const std::string& path)
{
  std::ifstream file(path, std::ios::ate | std::ios::binary);
  if (!file.is_open())
  {
    throw std::runtime_error("Failed to open SPIR-V file: " + path);
  }

  const size_t size = static_cast<size_t>(file.tellg());
  if (size == 0 || (size % sizeof(uint32_t)) != 0)
  {
    throw std::runtime_error("Invalid SPIR-V file size: " + path);
  }

  std::vector<uint32_t> buffer(size / sizeof(uint32_t));
  file.seekg(0);
  file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(size));

  return buffer;
}

}  // namespace eng