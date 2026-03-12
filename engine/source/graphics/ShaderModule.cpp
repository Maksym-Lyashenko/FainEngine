// VulkanUtils.cpp
#include "graphics/ShaderModule.h"

#include <glslang/Include/glslang_c_interface.h>
#include <lutils/ScopeExit.h>

#include <cassert>
#include <iomanip>
#include <iostream>
#include <string_view>

namespace eng
{

static glslang_stage_t getGLSLangShaderStage(const ShaderModule::ShaderStage stage)
{
  switch (stage)
  {
    case ShaderModule::Stage_Vert:
      return GLSLANG_STAGE_VERTEX;
    case ShaderModule::Stage_Tesc:
      return GLSLANG_STAGE_TESSCONTROL;
    case ShaderModule::Stage_Tese:
      return GLSLANG_STAGE_TESSEVALUATION;
    case ShaderModule::Stage_Geom:
      return GLSLANG_STAGE_GEOMETRY;
    case ShaderModule::Stage_Frag:
      return GLSLANG_STAGE_FRAGMENT;
    case ShaderModule::Stage_Comp:
      return GLSLANG_STAGE_COMPUTE;
    case ShaderModule::Stage_Task:
      return GLSLANG_STAGE_TASK;
    case ShaderModule::Stage_Mesh:
      return GLSLANG_STAGE_MESH;
    case ShaderModule::Stage_RayGen:
      return GLSLANG_STAGE_RAYGEN;
    case ShaderModule::Stage_AnyHit:
      return GLSLANG_STAGE_ANYHIT;
    case ShaderModule::Stage_ClosestHit:
      return GLSLANG_STAGE_CLOSESTHIT;
    case ShaderModule::Stage_Miss:
      return GLSLANG_STAGE_MISS;
    case ShaderModule::Stage_Intersection:
      return GLSLANG_STAGE_INTERSECT;
    case ShaderModule::Stage_Callable:
      return GLSLANG_STAGE_CALLABLE;
    default:
      assert(false);
      return GLSLANG_STAGE_COUNT;
  }
}

Result ShaderModule::compileShaderGlslang(
    const ShaderModule::ShaderStage stage,
    const char* code,
    std::vector<uint8_t>* outSPIRV,
    const ::glslang_resource_t* glslLangResource)
{
  if (!outSPIRV)
  {
    return {Result::Code::ArgumentOutOfRange, "outSPIRV is null"};
  }

  if (!code)
  {
    return {Result::Code::ArgumentOutOfRange, "code is null"};
  }

  const glslang_input_t input = {
      .language = GLSLANG_SOURCE_GLSL,
      .stage = getGLSLangShaderStage(stage),
      .client = GLSLANG_CLIENT_VULKAN,
      .client_version = GLSLANG_TARGET_VULKAN_1_3,
      .target_language = GLSLANG_TARGET_SPV,
      .target_language_version = GLSLANG_TARGET_SPV_1_6,
      .code = code,
      .default_version = 100,
      .default_profile = GLSLANG_NO_PROFILE,
      .force_default_version_and_profile = false,
      .forward_compatible = false,
      .messages = GLSLANG_MSG_DEFAULT_BIT,
      .resource = glslLangResource,
  };

  glslang_shader_t* shader = glslang_shader_create(&input);
  SCOPE_EXIT
  {
    glslang_shader_delete(shader);
  };

  if (!glslang_shader_preprocess(shader, &input))
  {
    std::cout << "Shader preprocessing failed:\n";
    std::cout << glslang_shader_get_info_log(shader) << '\n';
    std::cout << glslang_shader_get_info_debug_log(shader) << '\n';
    logShaderSource(code);
    return {Result::Code::RuntimeError, "glslang_shader_preprocess() failed"};
  }

  if (!glslang_shader_parse(shader, &input))
  {
    std::cout << "Shader parsing failed:\n";
    std::cout << glslang_shader_get_info_log(shader) << '\n';
    std::cout << glslang_shader_get_info_debug_log(shader) << '\n';
    logShaderSource(glslang_shader_get_preprocessed_code(shader));
    return {Result::Code::RuntimeError, "glslang_shader_parse() failed"};
  }

  glslang_program_t* program = glslang_program_create();
  SCOPE_EXIT
  {
    glslang_program_delete(program);
  };

  glslang_program_add_shader(program, shader);

  if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
  {
    std::cout << "Shader linking failed:\n";
    std::cout << glslang_program_get_info_log(program) << '\n';
    std::cout << glslang_program_get_info_debug_log(program) << '\n';
    return {Result::Code::RuntimeError, "glslang_program_link() failed"};
  }

  glslang_spv_options_t options = {
      .generate_debug_info = true,
      .strip_debug_info = false,
      .disable_optimizer = false,
      .optimize_size = true,
      .disassemble = false,
      .validate = true,
      .emit_nonsemantic_shader_debug_info = false,
      .emit_nonsemantic_shader_debug_source = false,
      .compile_only = false,
      .optimize_allow_expanded_id_bound = false,
  };

  glslang_program_SPIRV_generate_with_options(program, input.stage, &options);

  if (const char* msg = glslang_program_SPIRV_get_messages(program); msg && *msg)
  {
    std::cout << msg << '\n';
  }

  const auto* spirvBytes = reinterpret_cast<const uint8_t*>(glslang_program_SPIRV_get_ptr(program));
  const size_t numBytes = glslang_program_SPIRV_get_size(program) * sizeof(uint32_t);

  outSPIRV->assign(spirvBytes, spirvBytes + numBytes);
  return {};
}

bool endsWith(const char* s, const char* part)
{
  const size_t sLength = strlen(s);
  const size_t partLength = strlen(part);
  return sLength < partLength ? false : strcmp(s + sLength - partLength, part) == 0;
}

ShaderModule::ShaderStage ShaderModule::ShaderStageFromFileName(const char* fileName)
{
  if (endsWith(fileName, ".vert"))
  {
    return ShaderStage::Stage_Vert;
  }

  if (endsWith(fileName, ".frag"))
  {
    return ShaderStage::Stage_Frag;
  }

  if (endsWith(fileName, ".geom"))
  {
    return ShaderStage::Stage_Geom;
  }

  if (endsWith(fileName, ".comp"))
  {
    return ShaderStage::Stage_Comp;
  }

  if (endsWith(fileName, ".tesc"))
  {
    return ShaderStage::Stage_Tesc;
  }

  if (endsWith(fileName, ".tese"))
  {
    return ShaderStage::Stage_Tese;
  }

  return ShaderStage::Stage_Vert;
}

void ShaderModule::logShaderSource(const char* text)
{
  if (!text)
  {
    std::cout << "<null shader source>\n";
    return;
  }

  uint32_t line = 0;
  const char* lineStart = text;
  const char* p = text;

  while (*p)
  {
    if (*p == '\n')
    {
      std::string_view sv(lineStart, static_cast<size_t>(p - lineStart));
      if (!sv.empty() && sv.back() == '\r')
      {
        sv.remove_suffix(1);
      }

      std::cout << '(' << std::setw(3) << ++line << ") " << sv << '\n';
      lineStart = p + 1;
    }

    ++p;
  }

  if (p != lineStart)
  {
    std::string_view sv(lineStart, static_cast<size_t>(p - lineStart));
    if (!sv.empty() && sv.back() == '\r')
    {
      sv.remove_suffix(1);
    }

    std::cout << '(' << std::setw(3) << ++line << ") " << sv << '\n';
  }

  std::cout << '\n';
}

std::string ShaderModule::readShaderFile(const char* fileName)
{
  FILE* file = fopen(fileName, "r");

  if (!file)
  {
    std::cout << "I/O error. Cannot open shader file '%s'\n" << fileName << std::endl;
    return std::string();
  }

  fseek(file, 0L, SEEK_END);
  const size_t bytesinfile = ftell(file);
  fseek(file, 0L, SEEK_SET);

  char* buffer = (char*)alloca(bytesinfile + 1);
  const size_t bytesread = fread(buffer, 1, bytesinfile, file);
  fclose(file);

  buffer[bytesread] = 0;

  static constexpr unsigned char BOM[] = {0xEF, 0xBB, 0xBF};

  if (bytesread > 3)
  {
    if (!memcmp(buffer, BOM, 3))
    {
      memset(buffer, ' ', 3);
    }
  }

  std::string code(buffer);

  while (code.find("#include ") != code.npos)
  {
    const auto pos = code.find("#include ");
    const auto p1 = code.find('<', pos);
    const auto p2 = code.find('>', pos);
    if (p1 == code.npos || p2 == code.npos || p2 <= p1)
    {
      std::cout << "Error while loading shader program: %s\n" << code.c_str() << std::endl;
      return std::string();
    }
    const std::string name = code.substr(p1 + 1, p2 - p1 - 1);
    const std::string include = readShaderFile(name.c_str());
    code.replace(pos, p2 - pos + 1, include.c_str());
  }

  return code;
}

}  // namespace eng