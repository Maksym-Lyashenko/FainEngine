#include "utils/Camera.h"

#include <glm/ext.hpp>

#include <algorithm>
#include <cmath>

namespace eng
{
namespace
{
float radToDeg(float radians)
{
  return radians * 57.2957795131f;
}

float degToRad(float degrees)
{
  return degrees * 0.01745329252f;
}
}  // namespace

CameraPositionerFirstPerson::CameraPositionerFirstPerson(
    const glm::vec3& position, const glm::vec3& target, const glm::vec3& up)
{
  lookAt(position, target, up);
}

void CameraPositionerFirstPerson::lookAt(
    const glm::vec3& position, const glm::vec3& target, const glm::vec3& up)
{
  m_position = position;
  m_worldUp = glm::normalize(up);

  const glm::vec3 dir = glm::normalize(target - position);
  m_pitch = radToDeg(std::asin(dir.y));
  m_yaw = radToDeg(std::atan2(dir.z, dir.x));

  updateVectors();
}

void CameraPositionerFirstPerson::setSpeed(const glm::vec3& speed)
{
  m_speed = speed;
}

void CameraPositionerFirstPerson::updateVectors()
{
  const float yawRad = degToRad(m_yaw);
  const float pitchRad = degToRad(m_pitch);

  glm::vec3 forward{};
  forward.x = std::cos(yawRad) * std::cos(pitchRad);
  forward.y = std::sin(pitchRad);
  forward.z = std::sin(yawRad) * std::cos(pitchRad);

  m_forward = glm::normalize(forward);
  m_right = glm::normalize(glm::cross(m_forward, m_worldUp));
  m_up = glm::normalize(glm::cross(m_right, m_forward));
}

void CameraPositionerFirstPerson::update(
    float deltaSeconds, const glm::vec2& mousePos, bool mousePressed)
{
  if (mousePressed)
  {
    if (m_firstMouseSample)
    {
      m_lastMousePos = mousePos;
      m_firstMouseSample = false;
    }

    const glm::vec2 delta = mousePos - m_lastMousePos;
    m_lastMousePos = mousePos;

    m_yaw += delta.x * m_mouseSensitivity;
    m_pitch += delta.y * m_mouseSensitivity;
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);

    updateVectors();
  }
  else
  {
    m_firstMouseSample = true;
  }

  glm::vec3 moveDir{0.0f};

  if (movement.forward)
  {
    moveDir += m_forward;
  }
  if (movement.backward)
  {
    moveDir -= m_forward;
  }
  if (movement.right)
  {
    moveDir += m_right;
  }
  if (movement.left)
  {
    moveDir -= m_right;
  }
  if (movement.up)
  {
    moveDir += m_worldUp;
  }
  if (movement.down)
  {
    moveDir -= m_worldUp;
  }

  if (glm::length(moveDir) > 0.0f)
  {
    moveDir = glm::normalize(moveDir);
  }

  const float speed = movement.fastSpeed ? (m_moveSpeed * m_fastMultiplier) : m_moveSpeed;
  m_position += moveDir * speed * deltaSeconds;
  m_position += m_speed * deltaSeconds;
}

glm::mat4 CameraPositionerFirstPerson::viewMatrix() const
{
  return glm::lookAt(m_position, m_position + m_forward, m_up);
}

}  // namespace eng