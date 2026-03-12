#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <array>
#include <cstddef>

namespace eng
{
struct VertexP3
{
  glm::vec3 position{};

  static constexpr std::array<VkVertexInputBindingDescription, 1> Bindings()
  {
    return {VkVertexInputBindingDescription{
        .binding = 0,
        .stride = sizeof(VertexP3),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    }};
  }

  static constexpr std::array<VkVertexInputAttributeDescription, 1> Attributes()
  {
    return {VkVertexInputAttributeDescription{
        .location = 0,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = static_cast<uint32_t>(offsetof(VertexP3, position)),
    }};
  }
};

struct VertexP3C3
{
  glm::vec3 position{};
  glm::vec3 color{};

  static constexpr std::array<VkVertexInputBindingDescription, 1> Bindings()
  {
    return {VkVertexInputBindingDescription{
        .binding = 0,
        .stride = sizeof(VertexP3C3),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    }};
  }

  static constexpr std::array<VkVertexInputAttributeDescription, 2> Attributes()
  {
    return {
        VkVertexInputAttributeDescription{
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = static_cast<uint32_t>(offsetof(VertexP3C3, position)),
        },
        VkVertexInputAttributeDescription{
            .location = 1,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = static_cast<uint32_t>(offsetof(VertexP3C3, color)),
        }};
  }
};

struct VertexP3N3UV2
{
  glm::vec3 position{};
  glm::vec3 normal{};
  glm::vec2 uv{};

  static constexpr std::array<VkVertexInputBindingDescription, 1> Bindings()
  {
    return {VkVertexInputBindingDescription{
        .binding = 0,
        .stride = sizeof(VertexP3N3UV2),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    }};
  }

  static constexpr std::array<VkVertexInputAttributeDescription, 3> Attributes()
  {
    return {
        VkVertexInputAttributeDescription{
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = static_cast<uint32_t>(offsetof(VertexP3N3UV2, position)),
        },
        VkVertexInputAttributeDescription{
            .location = 1,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = static_cast<uint32_t>(offsetof(VertexP3N3UV2, normal)),
        },
        VkVertexInputAttributeDescription{
            .location = 2,
            .binding = 0,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = static_cast<uint32_t>(offsetof(VertexP3N3UV2, uv)),
        }};
  }
};

}  // namespace eng