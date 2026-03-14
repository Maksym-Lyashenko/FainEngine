#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

namespace eng
{
struct PerFrameData
{
  glm::mat4 model{1.0f};
  glm::mat4 view{1.0f};
  glm::mat4 proj{1.0f};
  glm::vec4 cameraPos{0.0f, 0.0f, 0.0f, 1.0f};
};
}  // namespace eng