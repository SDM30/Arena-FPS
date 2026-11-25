// =========================================================================
// Quake-style player movement
// =========================================================================
#ifndef __game__QPlayerMovement__h__
#define __game__QPlayerMovement__h__

#include <physics/Physics.h>

#include <glm/glm.hpp>

namespace physics {
class CollisionWorld;
} // namespace physics

namespace game {
// Per-step input snapshot produced by main.cpp.
struct Input {
  glm::vec3 wishDir{0.0f};
  bool jumpPressed = false;
  bool jumpHeld = false;
  // Free-movement vertical controls (Space / Left Ctrl).
  bool moveUp = false;
  bool moveDown = false;
  // Hitscan aim snapshot.
  bool firePressed = false;
  glm::vec3 aimOrigin{0.0f};
  glm::vec3 aimDirection{0.0f, 0.0f, -1.0f};
};

// Tuning values inspired by Quake 1, scaled to metres.
struct MovementConfig {
  float maxSpeed = 8.0f;
  float groundAccelerate = 10.0f;
  float airAccelerate = 10.0f;
  float friction = 6.0f;
  float stopSpeed = 2.5f;
  float jumpSpeed = 6.9f;
  float maxFallSpeed = 50.0f;
  // Quake clamps the air-acceleration wish speed to a small value.
  float airWishSpeedCap = 0.8f;
  // Held jump re-jumps on landing.
  bool autoBhop = false;
  // Debug/free camera.
  bool freeMove = false;
  float freeMoveSpeed = 15.0f;
};

class QPlayerMovement {
public:
  MovementConfig Config;

  /**
   * Applies friction/acceleration/jump/gravity and resolves the resulting
   * displacement against the static collision world.
   */
  void Step(physics::Body &body, const Input &input,
            physics::CollisionWorld &collision, float dt) const;

private:
  void _FreeMove(physics::Body &body, const Input &input, float dt) const;
  void _ApplyFriction(physics::Body &body, float dt) const;
  void _Accelerate(physics::Body &body, const glm::vec3 &wishDir,
                   float wishSpeed, float accelerate, float dt) const;
  void _AirAccelerate(physics::Body &body, const glm::vec3 &wishDir,
                      float wishSpeed, float accelerate, float dt) const;
};
} // namespace game

#endif // __game__QPlayerMovement__h__

// eof - QPlayerMovement.h
