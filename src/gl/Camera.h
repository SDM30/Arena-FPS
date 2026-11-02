// =========================================================================
// First-person perspective camera (GLM, no input handling)
// =========================================================================
#ifndef __gl__Camera__h__
#define __gl__Camera__h__

#include <glm/glm.hpp>

namespace gl {
/**
 * Stores an eye position and Euler angles (degrees). Input is intentionally
 * not handled here: the game loop decides when the mouse moves the camera
 * (see the "mouse captured only while Playing" rule in PLAN.md).
 */
class Camera {
public:
  using Self = Camera;

public:
  Camera() = default;
  explicit Camera(const glm::vec3 &position, float yaw = -90.0f,
                  float pitch = 0.0f);

  Self &SetPosition(const glm::vec3 &position);
  const glm::vec3 &GetPosition() const { return (this->Position); }

  Self &SetAngles(float yaw, float pitch);
  Self &AddAngles(float deltaYaw, float deltaPitch);

  float GetYaw() const { return (this->Yaw); }
  float GetPitch() const { return (this->Pitch); }

  glm::vec3 GetForward() const;
  glm::vec3 GetRight() const;
  glm::vec3 GetUp() const;

  glm::mat4 GetViewMatrix() const;
  glm::mat4 GetProjectionMatrix(float aspect) const;
  glm::mat4 GetProjectionMatrix(int width, int height) const;

public:
  float Fov{70.0f};
  float Near{0.1f};
  float Far{500.0f};

private:
  glm::vec3 Position{0.0f, 1.7f, 0.0f};
  float Yaw{-90.0f};
  float Pitch{0.0f};
};
} // namespace gl

#endif // __gl__Camera__h__

// eof - Camera.h
