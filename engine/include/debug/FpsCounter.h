#pragma once

namespace eng
{
class FpsCounter
{
 public:
  explicit FpsCounter(float averagingPeriodSeconds = 0.5f)
      : m_averagingPeriodSeconds(averagingPeriodSeconds)
  {
  }

  void tick(float deltaSeconds)
  {
    if (deltaSeconds <= 0.0f)
    {
      return;
    }

    m_accumulatedTime += deltaSeconds;
    m_frameCount += 1;

    if (m_accumulatedTime >= m_averagingPeriodSeconds)
    {
      m_fps = static_cast<float>(m_frameCount) / m_accumulatedTime;
      m_accumulatedTime = 0.0f;
      m_frameCount = 0;
    }
  }

  float getFPS() const { return m_fps; }

  float getMs() const { return (m_fps > 0.0f) ? (1000.0f / m_fps) : 0.0f; }

 private:
  float m_averagingPeriodSeconds = 0.5f;
  float m_accumulatedTime = 0.0f;
  int m_frameCount = 0;
  float m_fps = 0.0f;
};

}  // namespace eng