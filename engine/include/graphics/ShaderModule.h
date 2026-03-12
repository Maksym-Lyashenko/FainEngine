#pragma once

#include "vk/VulkanUtils.h"

#include <cstdint>
#include <vector>
#include <string>

struct glslang_resource_s;
typedef struct glslang_resource_s glslang_resource_t;

namespace eng
{

class ShaderModule
{
 public:
  ShaderModule() = default;
  ~ShaderModule() = default;

  enum ShaderStage : uint8_t
  {
    Stage_Vert,
    Stage_Tesc,
    Stage_Tese,
    Stage_Geom,
    Stage_Frag,
    Stage_Comp,
    Stage_Task,
    Stage_Mesh,
    Stage_RayGen,
    Stage_AnyHit,
    Stage_ClosestHit,
    Stage_Miss,
    Stage_Intersection,
    Stage_Callable,
  };

  Result compileShaderGlslang(
      ShaderStage stage,
      const char* code,
      std::vector<uint8_t>* outSPIRV,
      const ::glslang_resource_t* glslLangResource = nullptr);

  ShaderStage ShaderStageFromFileName(const char* fileName);

  std::string readShaderFile(const char* fileName);

 private:
  void logShaderSource(const char* text);
};

}  // namespace eng