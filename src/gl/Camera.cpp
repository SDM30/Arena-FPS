// =========================================================================
// First-person perspective camera (GLM, no input handling)
// =========================================================================

#include <gl/Camera.h>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

// -------------------------------------------------------------------------
gl::Camera::Camera(const glm::vec3 &position, float yaw, float pitch)
    : Position(position), Yaw(yaw), Pitch(pitch) {}

// -------------------------------------------------------------------------
gl::Camera::Self &gl::Camera::SetPosition(const glm::vec3 &position) {
  this->Position = position;
  return (*this);
}

// -------------------------------------------------------------------------
gl::Camera::Self &gl::Camera::SetAngles(float yaw, float pitch) {
  this->Yaw = yaw;
  this->Pitch = std::clamp(pitch, -89.0f, 89.0f);
  return (*this);
}

// -------------------------------------------------------------------------
gl::Camera::Self &gl::Camera::AddAngles(float deltaYaw, float deltaPitch) {
  this->Yaw += deltaYaw;
  this->Pitch = std::clamp(this->Pitch + deltaPitch, -89.0f, 89.0f);
  return (*this);
}

// -------------------------------------------------------------------------
glm::vec3 gl::Camera::GetForward() const {
  const float yaw = glm::radians(this->Yaw);
  const float pitch = glm::radians(this->Pitch);
  const float cosPitch = std::cos(pitch);
  return (glm::normalize(glm::vec3(cosPitch * std::cos(yaw),
                                   std::sin(pitch),
                                   cosPitch * std::sin(yaw))));
}

// -------------------------------------------------------------------------
glm::vec3 gl::Camera::GetRight() const {
  return (glm::normalize(glm::cross(this->GetForward(), glm::vec3(0, 1, 0))));
}

// -------------------------------------------------------------------------
glm::vec3 gl::Camera::GetUp() const {
  return (glm::normalize(glm::cross(this->GetRight(), this->GetForward())));
}

// -------------------------------------------------------------------------
glm::mat4 gl::Camera::GetViewMatrix() const {
  return (glm::lookAt(this->Position, this->Position + this->GetForward(),
                      this->GetUp()));
}

// -------------------------------------------------------------------------
glm::mat4 gl::Camera::GetProjectionMatrix(float aspect) const {
  const float safeAspect = (aspect > 0.0f) ? aspect : 1.0f;
  return (glm::perspective(glm::radians(this->Fov), safeAspect, this->Near,
                           this->Far));
}

// -------------------------------------------------------------------------
glm::mat4 gl::Camera::GetProjectionMatrix(int width, int height) const {
  return (this->GetProjectionMatrix(
      float(width) / float((height != 0) ? height : 1)));
}

// eof - Camera.cpp
