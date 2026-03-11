#pragma once

#include "vk/VulkanContext.h"
#include "vk/VulkanResourceHolder.h"

#include <string>

namespace eng
{

namespace ShaderLibrary
{

inline Holder<ShaderModuleHandle> LoadModule(IContext& ctx, const std::string& spvPath)
{
  return Holder<ShaderModuleHandle>(&ctx, loadShaderModule(&ctx, spvPath));
}

}  // namespace ShaderLibrary

}  // namespace eng