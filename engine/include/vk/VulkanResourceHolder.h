#pragma once

#include "vk/VulkanCommon.h"
#include "vk/VulkanContext.h"

#include <utility>

namespace eng
{
template <typename HandleT>
class Holder;

template <>
class Holder<ShaderModuleHandle>
{
 public:
  Holder() = default;

  Holder(IContext* ctx, ShaderModuleHandle handle) : m_ctx(ctx), m_handle(handle) {}

  ~Holder() { reset(); }

  Holder(const Holder&) = delete;
  Holder& operator=(const Holder&) = delete;

  Holder(Holder&& other) noexcept : m_ctx(other.m_ctx), m_handle(other.m_handle)
  {
    other.m_ctx = nullptr;
    other.m_handle = ShaderModuleHandle{};
  }

  Holder& operator=(Holder&& other) noexcept
  {
    if (this != &other)
    {
      reset();

      m_ctx = other.m_ctx;
      m_handle = other.m_handle;

      other.m_ctx = nullptr;
      other.m_handle = ShaderModuleHandle{};
    }

    return *this;
  }

  void reset()
  {
    if (m_ctx != nullptr && m_handle.valid())
    {
      m_ctx->destroyShaderModule(m_handle);
    }

    m_ctx = nullptr;
    m_handle = kInvalidHandle;
  }

  ShaderModuleHandle get() const { return m_handle; }
  ShaderModuleHandle release()
  {
    const ShaderModuleHandle h = m_handle;
    m_ctx = nullptr;
    m_handle = kInvalidHandle;
    return h;
  }

  bool valid() const { return m_handle.valid(); }

  operator ShaderModuleHandle() const { return m_handle; }

 private:
  IContext* m_ctx = nullptr;
  ShaderModuleHandle m_handle{};
};

template <>
class Holder<RenderPipelineHandle>
{
 public:
  Holder() = default;

  Holder(IContext* ctx, RenderPipelineHandle handle) : m_ctx(ctx), m_handle(handle) {}

  ~Holder() { reset(); }

  Holder(const Holder&) = delete;
  Holder& operator=(const Holder&) = delete;

  Holder(Holder&& other) noexcept : m_ctx(other.m_ctx), m_handle(other.m_handle)
  {
    other.m_ctx = nullptr;
    other.m_handle = RenderPipelineHandle{};
  }

  Holder& operator=(Holder&& other) noexcept
  {
    if (this != &other)
    {
      reset();

      m_ctx = other.m_ctx;
      m_handle = other.m_handle;

      other.m_ctx = nullptr;
      other.m_handle = RenderPipelineHandle{};
    }

    return *this;
  }

  void reset()
  {
    if (m_ctx != nullptr && m_handle.valid())
    {
      m_ctx->destroyRenderPipeline(m_handle);
    }

    m_ctx = nullptr;
    m_handle = kInvalidHandle;
  }

  RenderPipelineHandle get() const { return m_handle; }
  RenderPipelineHandle release()
  {
    const RenderPipelineHandle h = m_handle;
    m_ctx = nullptr;
    m_handle = kInvalidHandle;
    return h;
  }

  bool valid() const { return m_handle.valid(); }

  operator RenderPipelineHandle() const { return m_handle; }

 private:
  IContext* m_ctx = nullptr;
  RenderPipelineHandle m_handle = kInvalidHandle;
};

}  // namespace eng