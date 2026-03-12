#pragma once

#include <vulkan/vulkan.h>
#include "debug/Profiler.h"

#if defined(ENG_ENABLE_TRACY)
#include <tracy/TracyVulkan.hpp>

#define ENG_PROFILE_VK_ZONE(ctx, cmdBuf, name) \
  TracyVkNamedZone((ctx), ENG_CONCAT(__tracy_vk_zone_, __LINE__), (cmdBuf), name, true)
#else
namespace tracy
{
class VkCtx;
}
#define ENG_PROFILE_VK_ZONE(ctx, cmdBuf, name)
#endif