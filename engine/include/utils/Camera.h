#pragma once

#include <glm/glm.hpp>

namespace eng
{
class CameraPositionerFirstPerson
{
 public:
  struct Movement
  {
    bool forward = false;
    bool backward = false;
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
    bool fastSpeed = false;
  };

 public:
  CameraPositionerFirstPerson(
      const glm::vec3& position, const glm::vec3& target, const glm::vec3& up);

  void update(float deltaSeconds, const glm::vec2& mousePos, bool mousePressed);

  void lookAt(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up);
  void setSpeed(const glm::vec3& speed);

  glm::vec3 position() const { return m_position; }
  glm::vec3 forward() const { return m_forward; }
  glm::vec3 right() const { return m_right; }
  glm::vec3 up() const { return m_up; }

  glm::mat4 viewMatrix() const;

 public:
  Movement movement;

 private:
  void updateVectors();

 private:
  glm::vec3 m_position{0.0f};
  glm::vec3 m_forward{0.0f, 0.0f, 1.0f};
  glm::vec3 m_right{1.0f, 0.0f, 0.0f};
  glm::vec3 m_up{0.0f, 1.0f, 0.0f};
  glm::vec3 m_worldUp{0.0f, 1.0f, 0.0f};

  glm::vec3 m_speed{0.0f};

  float m_yaw = 0.0f;
  float m_pitch = 0.0f;

  glm::vec2 m_lastMousePos{0.0f};
  bool m_firstMouseSample = true;

  float m_moveSpeed = 2.5f;
  float m_fastMultiplier = 4.0f;
  float m_mouseSensitivity = 180.0f;
};

class Camera
{
 public:
  explicit Camera(CameraPositionerFirstPerson& positioner) : m_positioner(positioner) {}

  glm::vec3 getPosition() const { return m_positioner.position(); }
  glm::mat4 getViewMatrix() const { return m_positioner.viewMatrix(); }

 private:
  CameraPositionerFirstPerson& m_positioner;
};

}  // namespace eng