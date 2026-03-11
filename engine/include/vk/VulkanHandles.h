#pragma once

#include <cstdint>

namespace eng
{
struct TextureHandle
{
  uint32_t id = 0xFFFFFFFFu;
  bool IsValid() const { return id != 0xFFFFFFFFu; }
};

struct ShaderModuleHandle
{
  uint32_t id = 0xFFFFFFFFu;
  bool IsValid() const { return id != 0xFFFFFFFFu; }
};

struct RenderPipelineHandle
{
  uint32_t id = 0xFFFFFFFFu;
  bool IsValid() const { return id != 0xFFFFFFFFu; }
};

struct BufferHandle
{
  uint32_t id = 0xFFFFFFFFu;
  bool IsValid() const { return id != 0xFFFFFFFFu; }
};

}  // namespace eng