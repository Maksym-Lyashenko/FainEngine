#include "vulkan/ShaderModule.h"
#include "vulkan/VulkanUtils.h"

#include "eng.h"

#include <glslang/Include/glslang_c_interface.h>
#include "glslang/Public/resource_limits_c.h"

#include <cassert>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>

namespace
{

std::string readTextFile(const char* filename)
{
  std::ifstream file(filename, std::ios::in | std::ios::binary);
  if (!file)
  {
    return {};
  }

  file.seekg(0, std::ios::end);
  const std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  if (size <= 0)
  {
    return {};
  }

  std::string contents(static_cast<size_t>(size), '\0');
  file.read(contents.data(), size);
  return contents;
}

void saveSPIRVBinaryFile(const char* filename, const uint8_t* code, size_t size)
{
  std::ofstream file(filename, std::ios::out | std::ios::binary);
  if (!file)
  {
    std::cout << "ERROR!" << std::endl;
    return;
  }

  file.write(reinterpret_cast<const char*>(code), static_cast<std::streamsize>(size));
  std::cout << "Done!" << std::endl;
}

void testShaderCompilation(
    eng::ShaderModule& shaderManager,
    eng::ShaderModule::ShaderStage stage,
    const char* sourceFilename,
    const char* destFilename)
{
  const std::string shaderSource = readTextFile(sourceFilename);
  assert(!shaderSource.empty());

  std::vector<uint8_t> spirv;

  eng::Result res = shaderManager.compileShaderGlslang(
      stage, shaderSource.c_str(), &spirv, glslang_default_resource());

  (void)res;
  assert(!spirv.empty());

  saveSPIRVBinaryFile(destFilename, spirv.data(), spirv.size());
}

}  // namespace

int main()
{
  auto& shaderModule = eng::Engine::GetInstance().GetShaderModule();

  glslang_initialize_process();

  testShaderCompilation(
      shaderModule,
      eng::ShaderModule::Stage_Vert,
      "source/shaders/main.vert",
      ".cache/main.vert.spv");
  testShaderCompilation(
      shaderModule,
      eng::ShaderModule::Stage_Frag,
      "source/shaders/main.frag",
      ".cache/main.frag.spv");

  glslang_finalize_process();

  return 0;
}